import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1
import Qt.labs.qmlmodels 1.0


Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Print Settings")

    FileDialog {
        id: fileDialog
        title: qsTr("Выберите DOCX файл")
        nameFilters: ["Word documents (*.docx)"]
        onAccepted: {
            printerManager.docxFilePath = file.toString().replace("file:///", "")
        }
    }

    Popup {
        id: printPopup
        modal: true
        focus: true
        anchors.centerIn: parent
        width: 360
        height: 160

        Column {
            anchors.centerIn: parent
            spacing: 12

            Label {
                id: popupText
                wrapMode: Text.WordWrap
            }

            Button {
                text: "OK"
                onClicked: printPopup.close()
            }
        }
    }

    Connections {
        target: printerManager
        function onPrintResult(success, errorMessage) {
            popupText.text =
                success
                ? "Успех: " + errorMessage
                : "Ошибка: " + errorMessage

            printPopup.open()
        }
    }


    TabBar {
        id: tabBar
        width: parent.width
        anchors.top: parent.top

        TabButton {
            text: qsTr("Выбор файла")
        }
        TabButton {
            text: qsTr("Настройки печати")
        }
        TabButton {
            text: qsTr("БД")
        }
    }

    StackLayout {
        anchors.top: tabBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: tabBar.currentIndex

        // Вкладка 1: выбор DOCX файла
        Item {
            ColumnLayout {
                anchors.centerIn: parent
                width: parent.width - 40
                spacing: 20

                Label {
                    text: qsTr("Выберите документ Word для печати")
                    font.bold: true
                    font.pointSize: 12
                    Layout.alignment: Qt.AlignHCenter
                }

                RowLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: docxPathField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Путь к DOCX файлу")
                        text: printerManager.docxFilePath
                        onTextChanged: printerManager.docxFilePath = text
                    }
                    Button {
                        text: qsTr("Обзор...")
                        onClicked: fileDialog.open()
                    }
                }

                RowLayout {
                    visible: printerManager.conversionInProgress
                    BusyIndicator {
                        running: true
                        implicitWidth: 20
                        implicitHeight: 20
                    }
                    Label {
                        text: qsTr("Конвертация в PDF...")
                    }
                }

                Label {
                    id: conversionStatus
                    visible: !printerManager.conversionInProgress && printerManager.pdfFilePath.length > 0
                    text: qsTr("PDF готов: ") + printerManager.pdfFilePath
                    color: "green"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        // Вкладка 2: настройки печати
        ScrollView {
            contentWidth: availableWidth
            contentHeight: column.implicitHeight

            ColumnLayout {
                id: column
                width: parent.availableWidth
                spacing: 20

                RowLayout {
                    Layout.fillWidth: true
                    ComboBox {
                        id: printerCombo
                        Layout.fillWidth: true
                        model: printerManager.printerModel
                        textRole: "name"
                        currentIndex: printerManager.currentPrinterIndex
                        onCurrentIndexChanged: {
                            printerManager.currentPrinterIndex = currentIndex
                        }
                    }
                    Button {
                        text: "⟳"
                        onClicked: printerManager.refreshPrinters()
                    }
                }

                ColumnLayout {
                    visible: printerCombo.currentIndex >= 0
                    spacing: 15
                    Layout.fillWidth: true

                    ComboBox {
                        id: pageFormatCombo
                        Layout.fillWidth: true
                        model: printerManager.paperFormatsModel
                        textRole: "name"
                        currentIndex: printerManager.currentPaperFormatIndex
                        onCurrentIndexChanged: {
                            printerManager.currentPaperFormatIndex = currentIndex
                        }
                    }

                    ComboBox {
                        id: colorModeCombo
                        Layout.fillWidth: true
                        model: printerManager.colorModesModel
                        textRole: "name"
                        currentIndex: printerManager.currentColorModeIndex
                        onCurrentIndexChanged: {
                            printerManager.currentColorModeIndex = currentIndex
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Диапазон страниц:") }
                        TextField {
                            id: pageRangeField
                            Layout.fillWidth: true
                            placeholderText: qsTr("например, 1-10")
                            validator: RegularExpressionValidator {
                                regularExpression: /^\s*\d+\s*-\s*\d+\s*$|^\s*\d+\s*$|^\s*$/
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: qsTr("Количество копий:") }
                        SpinBox {
                            id: copiesSpin
                            from: 1
                            to: 99
                            value: 1
                            editable: true
                        }
                    }

                    Button {
                        text: qsTr("Печать")
                        Layout.alignment: Qt.AlignHCenter
                        enabled: printerCombo.currentIndex >= 0 &&
                                 pageFormatCombo.currentIndex >= 0 &&
                                 colorModeCombo.currentIndex >= 0 &&
                                 isValidPageRange(pageRangeField.text) &&
                                 copiesSpin.value > 0 &&
                                 printerManager.pdfFilePath.length > 0
                        onClicked: {
                            printerManager.printJob(pageRangeField.text, copiesSpin.value)
                        }
                    }
                }
            }
        }

        // Вкладка 3: таблица записей из БД
        Item {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                RowLayout {
                    Layout.alignment: Qt.AlignLeft
                    spacing: 8
                    Button {
                        text: qsTr("Токен")
                        onClicked: {
                            var url = "https://www.dropbox.com/oauth2/authorize?response_type=code&token_access_type=offline&client_id=9so3tgijjtzer5l"
                            Qt.openUrlExternally(url)
                            text: printerManager.code
                            onTextChanged: printerManager.code = text
                        }
                    }
                    TextField {
                        id: codeField
                        Layout.fillWidth: true
                        placeholderText: qsTr("Код аунтификации")
                        text: printerManager.code
                        onTextChanged: printerManager.code = text
                    }
                    Item { Layout.fillWidth: true }


                }

                RowLayout {
                    Layout.alignment: Qt.AlignLeft
                    spacing: 8
                    Button {
                        text: qsTr("Обновить")
                        onClicked: {
                            printerManager.refreshPrintJobsFromRemote()
                        }
                    }
                    Label {
                        text: qsTr("Записи из базы печати")
                        font.bold: true
                        Layout.alignment: Qt.AlignVCenter
                    }
                    Item { Layout.fillWidth: true }


                }



                Rectangle {
                    color: "transparent"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.width: 0



                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        HorizontalHeaderView {
                            id: headerView
                            Layout.fillWidth: true
                            Layout.preferredHeight: 50
                            syncView: tableView
                            model: ["Пользователь", "Принтер", "Страниц"]

                            delegate: Rectangle {
                                implicitWidth: {
                                    if (index === 0) return 150
                                    if (index === 1) return 250
                                    return 80
                                }
                                implicitHeight: 30
                                height: 30
                                color: "#e0e0e0"
                                border.width: 1
                                border.color: "#aaa"

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData
                                    font.bold: true
                                    font.pixelSize: 14
                                }
                            }
                        }

                        TableView {
                            id: tableView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: printerManager.printJobsModel

                            clip: true

                            delegate: Rectangle {
                                implicitHeight: 40
                                implicitWidth: {
                                    if (column === 0) return 150
                                    if (column === 1) return 250
                                    return 80
                                }

                                color: row % 2 === 0 ? "#f5f5f5" : "white"
                                border.width: 1
                                border.color: "#ddd"

                                Text {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    text: display     // ← теперь так
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: column === 2
                                                         ? Text.AlignRight
                                                         : Text.AlignLeft
                                    font.pixelSize: 14
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: printerManager
        function onConversionFinished(success, errorString) {
            if (!success) {
                conversionStatus.text = qsTr("Ошибка конвертации: ") + errorString
                conversionStatus.color = "red"
                conversionStatus.visible = true
            }
        }
    }

    Connections {
        target: printerManager
        function onPrintResult(success, errorMessage) {
            printMessageDialog.text = success ? qsTr("Успех: ") + errorMessage : qsTr("Ошибка: ") + errorMessage
            printMessageDialog.open()
        }
    }

    function isValidPageRange(range) {
        var trimmed = range.trim();
        if (trimmed.length === 0) return false;

        var singleMatch = /^\d+$/.test(trimmed);
        if (singleMatch) {
            var num = parseInt(trimmed, 10);
            return num > 0;
        }

        var rangeMatch = /^(\d+)\s*-\s*(\d+)$/.exec(trimmed);
        if (rangeMatch) {
            var start = parseInt(rangeMatch[1], 10);
            var end = parseInt(rangeMatch[2], 10);
            return start > 0 && end > 0 && start <= end;
        }

        return false;
    }
}
