#include "printermanager.h"
#include "params.h"
#include "DOCXtoPDF.h"
#include "db_manager.h"
#include <QDebug>
#include <QFileInfo>
#include <QUuid>
#include <QRegularExpression>
#include <QStandardPaths>
#include <vector>


static QString fromStdWString(const std::wstring &ws) {
    return QString::fromStdWString(ws);
}

// ---------- PrinterListModel ----------
PrinterListModel::PrinterListModel(QObject *parent)
    : QAbstractListModel(parent) {}

int PrinterListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_printers.size();
}

QVariant PrinterListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_printers.size())
        return QVariant();
    if (role == NameRole)
        return m_printers.at(index.row());
    return QVariant();
}

QHash<int, QByteArray> PrinterListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    return roles;
}

void PrinterListModel::setPrinters(const QList<QString> &printers) {
    beginResetModel();
    m_printers = printers;
    endResetModel();
}

void PrinterListModel::clear() {
    beginResetModel();
    m_printers.clear();
    endResetModel();
}

// ---------- NamedCodeModel ----------
NamedCodeModel::NamedCodeModel(QObject *parent)
    : QAbstractListModel(parent) {}

int NamedCodeModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant NamedCodeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size())
        return QVariant();
    const auto &item = m_items.at(index.row());
    if (role == NameRole)
        return item.first;
    else if (role == CodeRole)
        return item.second;
    return QVariant();
}

QHash<int, QByteArray> NamedCodeModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[CodeRole] = "code";
    return roles;
}

void NamedCodeModel::setItems(const QList<QPair<QString, int>> &items) {
    beginResetModel();
    m_items = items;
    endResetModel();
}

void NamedCodeModel::clear() {
    beginResetModel();
    m_items.clear();
    endResetModel();
}

// ---------- PrinterManager ----------
PrinterManager::PrinterManager(QObject *parent)
    : QObject(parent)
    , m_printerModel(new PrinterListModel(this))
    , m_paperFormatsModel(new NamedCodeModel(this))
    , m_colorModesModel(new NamedCodeModel(this))
    , m_printJobsModel(new PrintJobsModel(this))
    , m_currentPrinterIndex(-1)
    , m_currentPaperFormatIndex(-1)
    , m_currentColorModeIndex(-1)
    , m_conversionInProgress(false)
{
    refreshPrinters();
}

void PrinterManager::refreshPrintJobsFromRemote() {
    QString localDbPath = QDir::temp().absoluteFilePath("printjobs.db");
    std::vector<PrintJob> jobsVec;

    bool ok = false;
    try {
        if (!m_code.isEmpty())
            ok = readPrintJobs(m_code.toStdString(), jobsVec, localDbPath.toStdString());
    } catch (const std::exception &e) {
        qWarning() << "Exception in readPrintJobs:" << e.what();
        ok = false;
    } catch (...) {
        qWarning() << "Unknown exception in readPrintJobs";
        ok = false;
    }

    if (!ok) {
        emit printResult(false, tr("Не удалось загрузить или прочитать базу записей печати."));
        m_printJobsModel->clear();
        return;
    }

    QList<PrintJobsModel::Item> items;
    items.reserve(static_cast<int>(jobsVec.size()));
    for (const auto &pj : jobsVec) {
        PrintJobsModel::Item it;
        it.user = QString::fromStdString(pj.user);
        it.printer = QString::fromStdString(pj.printer);
        it.pages = pj.pages;
        items.append(it);
    }

    m_printJobsModel->setJobs(items);
    emit printResult(true, tr("Список заданий обновлён. Всего: %1").arg(items.size()));
}

void PrinterManager::setCurrentPrinterIndex(int index) {
    if (m_currentPrinterIndex == index)
        return;
    m_currentPrinterIndex = index;
    emit currentPrinterIndexChanged();

    if (index >= 0 && index < m_printerModel->rowCount()) {
        QString printerName = m_printerModel->data(m_printerModel->index(index), PrinterListModel::NameRole).toString();
        updatePaperFormatsAndColorModes(printerName);
    } else {
        m_paperFormatsModel->clear();
        m_colorModesModel->clear();
    }
}

void PrinterManager::setCurrentPaperFormatIndex(int index) {
    if (m_currentPaperFormatIndex == index)
        return;
    m_currentPaperFormatIndex = index;
    emit currentPaperFormatIndexChanged();
}

void PrinterManager::setCurrentColorModeIndex(int index) {
    if (m_currentColorModeIndex == index)
        return;
    m_currentColorModeIndex = index;
    emit currentColorModeIndexChanged();
}

void PrinterManager::refreshPrinters() {
    std::vector<std::wstring> printerNames = GetPrinterNames();
    QList<QString> names;
    for (const auto &ws : printerNames)
        names.append(fromStdWString(ws));
    m_printerModel->setPrinters(names);

    if (m_currentPrinterIndex >= m_printerModel->rowCount())
        setCurrentPrinterIndex(-1);
    else if (m_currentPrinterIndex >= 0)
        setCurrentPrinterIndex(m_currentPrinterIndex);
    else {
        m_paperFormatsModel->clear();
        m_colorModesModel->clear();
    }
}

void PrinterManager::updatePaperFormatsAndColorModes(const QString &printerName) {
    std::wstring wName = printerName.toStdWString();

    auto paperFormats = GetPaperFormatsWithCodes(wName.c_str());
    QList<QPair<QString, int>> paperItems;
    for (const auto &p : paperFormats)
        paperItems.append({fromStdWString(p.first), static_cast<int>(p.second)});
    m_paperFormatsModel->setItems(paperItems);

    auto colorModes = GetColorModesWithCodes(wName.c_str());
    QList<QPair<QString, int>> colorItems;
    for (const auto &p : colorModes)
        colorItems.append({fromStdWString(p.first), static_cast<int>(p.second)});
    m_colorModesModel->setItems(colorItems);

    setCurrentPaperFormatIndex(paperItems.isEmpty() ? -1 : 0);
    setCurrentColorModeIndex(colorItems.isEmpty() ? -1 : 0);
}

void PrinterManager::setDocxFilePath(const QString &path) {
    if (m_docxFilePath == path)
        return;
    m_docxFilePath = path;
    emit docxFilePathChanged();

    if (!path.isEmpty())
        convertDocxToPdf(path);
}

void PrinterManager::setCode(const QString &code) {
    if (m_code == code)
        return;
    m_code = code;
    emit codeChanged();

    // if (!code.isEmpty())
    //     qDebug() << code;
}



void PrinterManager::cleanupPdfFile() {
    if (!m_pdfFilePath.isEmpty()) {
        QFile file(m_pdfFilePath);
        if (file.exists())
            file.remove();
        m_pdfFilePath.clear();
        emit pdfFilePathChanged();
    }
}

bool PrinterManager::convertDocxToPdf(const QString &docxPath) {
    if (docxPath.isEmpty()) {
        emit conversionFinished(false, tr("Путь к DOCX файлу не указан."));
        return false;
    }

    QFileInfo fi(docxPath);
    if (!fi.exists()) {
        emit conversionFinished(false, tr("Файл не существует: ") + docxPath);
        return false;
    }

    cleanupPdfFile();

    QString tempDir = QDir::temp().absolutePath();
    QString pdfFileName = QUuid::createUuid().toString(QUuid::WithoutBraces) + ".pdf";
    QString pdfPath = tempDir + "/" + pdfFileName;

    m_conversionInProgress = true;
    emit conversionInProgressChanged();

    bool success = false;
    QString errorMsg;
    try {
        convertWordToPDF(docxPath.toStdString(), pdfPath.toStdString());
        success = true;
    } catch (const std::exception &e) {
        errorMsg = QString::fromStdString(e.what());
    } catch (...) {
        errorMsg = tr("Неизвестная ошибка при конвертации.");
    }

    if (success) {
        m_pdfFilePath = pdfPath;
        emit pdfFilePathChanged();
        qDebug() << "PDF создан:" << pdfPath;
    } else {
        qWarning() << "Ошибка конвертации:" << errorMsg;
    }

    m_conversionInProgress = false;
    emit conversionInProgressChanged();
    emit conversionFinished(success, errorMsg);

    return success;
}

void PrinterManager::printJob(const QString &pageRange, int copies) {
    qDebug() << "printJob called. pageRange:" << pageRange << "copies:" << copies;

    if (m_currentPrinterIndex < 0 || m_currentPrinterIndex >= m_printerModel->rowCount()) {
        emit printResult(false, tr("Принтер не выбран."));
        return;
    }

    if (m_pdfFilePath.isEmpty()) {
        emit printResult(false, tr("PDF файл не готов."));
        return;
    }

    if (copies <= 0) {
        emit printResult(false, tr("Неверное количество копий."));
        return;
    }

    QRegularExpression re(R"((\d+)\s*-\s*(\d+))");
    QRegularExpressionMatch match = re.match(pageRange.trimmed());
    int start = -1, end = -1;
    if (match.hasMatch()) {
        start = match.captured(1).toInt();
        end = match.captured(2).toInt();
    } else {
        QRegularExpression reSingle(R"(^\s*(\d+)\s*$)");
        QRegularExpressionMatch m2 = reSingle.match(pageRange);
        if (m2.hasMatch()) {
            start = m2.captured(1).toInt();
            end = start;
        } else {
            emit printResult(false, tr("Неверный формат диапазона страниц. Используйте например: 1-10 или 5."));
            return;
        }
    }

    if (start <= 0 || end <= 0) {
        emit printResult(false, tr("Номера страниц должны быть положительными."));
        return;
    }
    if (end < start) {
        emit printResult(false, tr("Начало диапазона не может быть больше конца."));
        return;
    }

    long long pagesPerCopy = static_cast<long long>(end) - static_cast<long long>(start) + 1;
    if (pagesPerCopy <= 0) {
        emit printResult(false, tr("Некорректное количество страниц."));
        return;
    }

    long long totalPagesLL = pagesPerCopy * static_cast<long long>(copies);
    if (totalPagesLL > std::numeric_limits<int>::max()) {
        emit printResult(false, tr("Слишком большое количество страниц."));
        return;
    }
    int totalPages = static_cast<int>(totalPagesLL);

    QString printerName = m_printerModel->data(m_printerModel->index(m_currentPrinterIndex), PrinterListModel::NameRole).toString();

    QString localDbPath = QDir::temp().absoluteFilePath("printjobs.db");


    bool ok = false;
    try {
        if (!m_code.isEmpty())
            ok = addPrintJob(m_code.toStdString(), printerName.toStdString(), totalPages, localDbPath.toStdString());
    } catch (const std::exception &e) {
        emit printResult(false, tr("Исключение при добавлении в БД: ") + QString::fromStdString(e.what()));
        return;
    } catch (...) {
        emit printResult(false, tr("Неизвестное исключение при добавлении в БД."));
        return;
    }

    if (ok) {
        emit printResult(true, tr("Задание на печать успешно добавлено."));
        // Здесь можно добавить реальную отправку на принтер
    } else {
        emit printResult(false, tr("Ошибка при добавлении записи в БД."));
    }
}


// ---------- PrintJobsModel ----------
PrintJobsModel::PrintJobsModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int PrintJobsModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_jobs.size();
}

int PrintJobsModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return 3; // user | printer | pages
}

QVariant PrintJobsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    const Item &it = m_jobs.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return it.user;
        case 1: return it.printer;
        case 2: return it.pages;
        default: return QVariant();
        }
    }

    return QVariant();
}

QVariant PrintJobsModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return "Пользователь";
        case 1: return "Принтер";
        case 2: return "Страниц";
        }
    }

    return QVariant();
}

void PrintJobsModel::setJobs(const QList<Item> &jobs) {
    beginResetModel();
    m_jobs = jobs;
    endResetModel();
}

void PrintJobsModel::clear() {
    beginResetModel();
    m_jobs.clear();
    endResetModel();
}
