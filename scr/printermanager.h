#ifndef PRINTERMANAGER_H
#define PRINTERMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <QList>
#include <QPair>
#include <QString>
#include <QFile>
#include <QDir>


class PrinterListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles { NameRole = Qt::UserRole + 1 };
    explicit PrinterListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setPrinters(const QList<QString> &printers);
    void clear();

private:
    QList<QString> m_printers;
};


class NamedCodeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles { NameRole = Qt::UserRole + 1, CodeRole };
    explicit NamedCodeModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setItems(const QList<QPair<QString, int>> &items);
    void clear();

private:
    QList<QPair<QString, int>> m_items;
};


class PrintJobsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Roles { UserRole = Qt::UserRole + 1, PrinterRole, PagesRole };

    struct Item {
        QString user;
        QString printer;
        int pages;
    };

    explicit PrintJobsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setJobs(const QList<Item> &jobs);
    void clear();

private:
    QList<Item> m_jobs;
};


class PrinterManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PrinterListModel* printerModel READ printerModel CONSTANT)
    Q_PROPERTY(NamedCodeModel* paperFormatsModel READ paperFormatsModel CONSTANT)
    Q_PROPERTY(NamedCodeModel* colorModesModel READ colorModesModel CONSTANT)
    Q_PROPERTY(int currentPrinterIndex READ currentPrinterIndex WRITE setCurrentPrinterIndex NOTIFY currentPrinterIndexChanged)
    Q_PROPERTY(int currentPaperFormatIndex READ currentPaperFormatIndex WRITE setCurrentPaperFormatIndex NOTIFY currentPaperFormatIndexChanged)
    Q_PROPERTY(int currentColorModeIndex READ currentColorModeIndex WRITE setCurrentColorModeIndex NOTIFY currentColorModeIndexChanged)
    Q_PROPERTY(QString docxFilePath READ docxFilePath WRITE setDocxFilePath NOTIFY docxFilePathChanged)
    Q_PROPERTY(QString code READ code WRITE setCode NOTIFY codeChanged)
    Q_PROPERTY(QString pdfFilePath READ pdfFilePath NOTIFY pdfFilePathChanged)
    Q_PROPERTY(bool conversionInProgress READ conversionInProgress NOTIFY conversionInProgressChanged)
    Q_PROPERTY(PrintJobsModel* printJobsModel READ printJobsModel CONSTANT)

public:
    explicit PrinterManager(QObject *parent = nullptr);

    PrinterListModel* printerModel() const { return m_printerModel; }
    NamedCodeModel* paperFormatsModel() const { return m_paperFormatsModel; }
    NamedCodeModel* colorModesModel() const { return m_colorModesModel; }
    PrintJobsModel* printJobsModel() const { return m_printJobsModel; }

    int currentPrinterIndex() const { return m_currentPrinterIndex; }
    void setCurrentPrinterIndex(int index);

    int currentPaperFormatIndex() const { return m_currentPaperFormatIndex; }
    void setCurrentPaperFormatIndex(int index);

    int currentColorModeIndex() const { return m_currentColorModeIndex; }
    void setCurrentColorModeIndex(int index);

    QString docxFilePath() const { return m_docxFilePath; }
    void setDocxFilePath(const QString &path);

    QString code() const { return m_code; }
    void setCode(const QString &path);


    QString pdfFilePath() const { return m_pdfFilePath; }
    bool conversionInProgress() const { return m_conversionInProgress; }

    Q_INVOKABLE void refreshPrinters();
    Q_INVOKABLE void printJob(const QString &pageRange, int copies);
    Q_INVOKABLE bool convertDocxToPdf(const QString &docxPath);

    Q_INVOKABLE void refreshPrintJobsFromRemote();

signals:
    void currentPrinterIndexChanged();
    void currentPaperFormatIndexChanged();
    void currentColorModeIndexChanged();
    void docxFilePathChanged();
    void codeChanged();
    void pdfFilePathChanged();
    void conversionInProgressChanged();
    void conversionFinished(bool success, const QString &errorString);
    void printResult(bool success, const QString &errorMessage);

private:
    void updatePaperFormatsAndColorModes(const QString &printerName);
    void cleanupPdfFile();

    PrinterListModel *m_printerModel;
    NamedCodeModel *m_paperFormatsModel;
    NamedCodeModel *m_colorModesModel;
    PrintJobsModel *m_printJobsModel;

    int m_currentPrinterIndex;
    int m_currentPaperFormatIndex;
    int m_currentColorModeIndex;

    QString m_docxFilePath;
    QString m_code;
    QString m_pdfFilePath;
    bool m_conversionInProgress;
};

#endif
