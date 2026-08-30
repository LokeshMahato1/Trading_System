import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic 2.15

Popup {
    id: root
    width: 380
    height: 340
    modal: true
    focus: true
    anchors.centerIn: parent
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property string prefillSymbol: ""

    background: Rectangle { color: "#0F1723"; radius: 12; border.color: "#1E2D40" }

    onOpened: {
        symbolField.text = prefillSymbol
        statusText.text = ""
    }

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Text { text: "Buy / Sell"; color: "#E2E8F0"; font.pixelSize: 16; font.bold: true }

        Text { text: "Symbol"; color: "#94A3B8"; font.pixelSize: 11 }
        TextField {
            id: symbolField
            width: parent.width
            placeholderText: "e.g. NABIL"
            color: "#E2E8F0"; placeholderTextColor: "#4A5568"
            background: Rectangle { color: "#0D1520"; radius: 7; border.color: "#1E2D40" }
            leftPadding: 12
        }

        Row {
            width: parent.width
            spacing: 10
            Column {
                width: (parent.width - 10) / 2
                spacing: 4
                Text { text: "Quantity"; color: "#94A3B8"; font.pixelSize: 11 }
                TextField {
                    id: qtyField
                    width: parent.width
                    placeholderText: "0"
                    color: "#E2E8F0"; placeholderTextColor: "#4A5568"
                    validator: IntValidator { bottom: 1 }
                    background: Rectangle { color: "#0D1520"; radius: 7; border.color: "#1E2D40" }
                    leftPadding: 12
                }
            }
            Column {
                width: (parent.width - 10) / 2
                spacing: 4
                Text { text: "Price"; color: "#94A3B8"; font.pixelSize: 11 }
                TextField {
                    id: priceField
                    width: parent.width
                    placeholderText: "0.00"
                    color: "#E2E8F0"; placeholderTextColor: "#4A5568"
                    validator: DoubleValidator { bottom: 0.01; decimals: 2 }
                    background: Rectangle { color: "#0D1520"; radius: 7; border.color: "#1E2D40" }
                    leftPadding: 12
                }
            }
        }

        Row {
            width: parent.width
            spacing: 10
            Rectangle {
                width: (parent.width - 10) / 2; height: 40; radius: 8
                color: buyArea.containsMouse ? "#2FB489" : "#34D399"
                Text { anchors.centerIn: parent; text: "Buy"; color: "#062015"; font.pixelSize: 13; font.bold: true }
                MouseArea {
                    id: buyArea
                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        var ok = engine.placeBuyOrder(symbolField.text.toUpperCase(), parseInt(qtyField.text || "0"), parseFloat(priceField.text || "0"))
                        statusText.text = ok ? "Buy order placed." : engine.lastError
                        statusText.color = ok ? "#34D399" : "#F87171"
                        if (ok) root.close()
                    }
                }
            }
            Rectangle {
                width: (parent.width - 10) / 2; height: 40; radius: 8
                color: sellArea.containsMouse ? "#D64158" : "#F87171"
                Text { anchors.centerIn: parent; text: "Sell"; color: "#2A0A0E"; font.pixelSize: 13; font.bold: true }
                MouseArea {
                    id: sellArea
                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        var ok = engine.placeSellOrder(symbolField.text.toUpperCase(), parseInt(qtyField.text || "0"), parseFloat(priceField.text || "0"))
                        statusText.text = ok ? "Sell order placed." : engine.lastError
                        statusText.color = ok ? "#34D399" : "#F87171"
                        if (ok) root.close()
                    }
                }
            }
        }

        Text {
            id: statusText
            width: parent.width
            wrapMode: Text.WordWrap
            font.pixelSize: 12
        }
    }
}
