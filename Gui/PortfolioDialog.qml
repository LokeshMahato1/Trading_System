import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic 2.15

Popup {
    id: root
    width: 600
    height: 440
    modal: true
    focus: true
    anchors.centerIn: parent
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle { color: "#0F1723"; radius: 12; border.color: "#1E2D40" }

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        Text { text: "Portfolio Holdings"; color: "#E2E8F0"; font.pixelSize: 16; font.bold: true }

        Row {
            width: parent.width
            Text { text: "Symbol"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.18 }
            Text { text: "Qty"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.12 }
            Text { text: "Avg Price"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.18 }
            Text { text: "Last Price"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.18 }
            Text { text: "Value"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.16 }
            Text { text: "P/L"; color: "#4A5568"; font.pixelSize: 11; width: parent.width * 0.18 }
        }
        Rectangle { width: parent.width; height: 1; color: "#1E2D40" }

        ListView {
            width: parent.width
            height: parent.height - 90
            clip: true
            model: engine.holdings
            delegate: Row {
                width: ListView.view.width
                height: 34
                Text { text: modelData.symbol; color: "#E2E8F0"; font.pixelSize: 12; font.bold: true; width: parent.width * 0.18; anchors.verticalCenter: parent.verticalCenter }
                Text { text: modelData.quantity; color: "#94A3B8"; font.pixelSize: 12; width: parent.width * 0.12; anchors.verticalCenter: parent.verticalCenter }
                Text { text: modelData.averagePrice.toFixed(2); color: "#E2E8F0"; font.pixelSize: 12; width: parent.width * 0.18; anchors.verticalCenter: parent.verticalCenter }
                Text { text: modelData.lastPrice.toFixed(2); color: "#E2E8F0"; font.pixelSize: 12; width: parent.width * 0.18; anchors.verticalCenter: parent.verticalCenter }
                Text { text: modelData.marketValue.toFixed(2); color: "#E2E8F0"; font.pixelSize: 12; width: parent.width * 0.16; anchors.verticalCenter: parent.verticalCenter }
                Text {
                    text: (modelData.profitLoss >= 0 ? "+" : "") + modelData.profitLoss.toFixed(2)
                    color: modelData.up ? "#34D399" : "#F87171"
                    font.pixelSize: 12; font.bold: true; width: parent.width * 0.18
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Text {
                anchors.centerIn: parent
                visible: parent.count === 0
                text: "No holdings yet — place a buy order to get started."
                color: "#4A5568"; font.pixelSize: 12
            }
        }
    }
}
