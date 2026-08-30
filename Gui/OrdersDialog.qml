import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic 2.15

Popup {
    id: root
    width: 560
    height: 420
    modal: true
    focus: true
    anchors.centerIn: parent
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle { color: "#0F1723"; radius: 12; border.color: "#1E2D40" }

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        Text { text: "My Orders"; color: "#E2E8F0"; font.pixelSize: 16; font.bold: true }

        Row {
            width: parent.width
            Text { text: "Symbol"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.18 }
            Text { text: "Type"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.14 }
            Text { text: "Qty (rem/total)"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.24 }
            Text { text: "Price"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.16 }
            Text { text: "Status"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.2 }
        }

        Rectangle { width: parent.width; height: 1; color: "#1E2D40" }

        ListView {
            width: parent.width
            height: parent.height - 90
            clip: true
            model: engine.myOrders
            delegate: Row {
                width: ListView.view.width
                height: 34
                Text { text: modelData.symbol; color: "#E2E8F0"; font.pixelSize: 12; width: parent.width * 0.18; anchors.verticalCenter: parent.verticalCenter }
                Text {
                    text: modelData.type; font.pixelSize: 12; font.bold: true; width: parent.width * 0.14
                    color: modelData.type === "BUY" ? "#34D399" : "#F87171"
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: modelData.remaining + " / " + modelData.quantity
                    color: "#94A3B8"; font.pixelSize: 12; width: parent.width * 0.24
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text { text: modelData.price.toFixed(2); color: "#E2E8F0"; font.pixelSize: 12; width: parent.width * 0.16; anchors.verticalCenter: parent.verticalCenter }
                Row {
                    width: parent.width * 0.28
                    spacing: 8
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        text: modelData.status
                        font.pixelSize: 11; font.bold: true
                        color: modelData.status === "FILLED" ? "#34D399"
                               : (modelData.status === "FAILED" || modelData.status === "CANCELLED") ? "#F87171"
                               : (modelData.status === "PARTIALLY_FILLED" ? "#F0B429" : "#5B8DEF")
                    }
                    Rectangle {
                        visible: modelData.isOpen
                        width: 60; height: 22; radius: 5
                        color: cancelArea.containsMouse ? "#D64158" : "#3D1515"
                        Text { anchors.centerIn: parent; text: "Cancel"; color: "#F87171"; font.pixelSize: 10 }
                        MouseArea {
                            id: cancelArea
                            anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: engine.cancelOrder(modelData.orderId)
                        }
                    }
                }
            }
        }
    }
}
