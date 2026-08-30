import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic 2.15

Popup {
    id: root
    width: 360
    height: 300
    modal: true
    focus: true
    anchors.centerIn: parent
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle { color: "#0F1723"; radius: 12; border.color: "#1E2D40" }

    onOpened: statusText.text = ""

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Text { text: "Collateral Transfer"; color: "#E2E8F0"; font.pixelSize: 16; font.bold: true }

        Row {
            width: parent.width
            Column {
                width: parent.width / 3
                Text { text: "Available"; color: "#64748B"; font.pixelSize: 10 }
                Text { text: "Rs " + engine.collateralAvailable.toFixed(2); color: "#34D399"; font.pixelSize: 13; font.bold: true }
            }
            Column {
                width: parent.width / 3
                Text { text: "Blocked"; color: "#64748B"; font.pixelSize: 10 }
                Text { text: "Rs " + engine.collateralBlocked.toFixed(2); color: "#F0B429"; font.pixelSize: 13; font.bold: true }
            }
            Column {
                width: parent.width / 3
                Text { text: "Total"; color: "#64748B"; font.pixelSize: 10 }
                Text { text: "Rs " + engine.collateralTotal.toFixed(2); color: "#E2E8F0"; font.pixelSize: 13; font.bold: true }
            }
        }

        Text { text: "Amount"; color: "#94A3B8"; font.pixelSize: 11 }
        TextField {
            id: amountField
            width: parent.width
            placeholderText: "0.00"
            color: "#E2E8F0"; placeholderTextColor: "#4A5568"
            validator: DoubleValidator { bottom: 0.01; decimals: 2 }
            background: Rectangle { color: "#0D1520"; radius: 7; border.color: "#1E2D40" }
            leftPadding: 12
        }

        Row {
            width: parent.width
            spacing: 10
            Rectangle {
                width: (parent.width - 10) / 2; height: 40; radius: 8
                color: depositArea.containsMouse ? "#2FB489" : "#34D399"
                Text { anchors.centerIn: parent; text: "Deposit"; color: "#062015"; font.pixelSize: 13; font.bold: true }
                MouseArea {
                    id: depositArea
                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        var ok = engine.depositCollateral(parseFloat(amountField.text || "0"))
                        statusText.text = ok ? "Deposited." : engine.lastError
                        statusText.color = ok ? "#34D399" : "#F87171"
                    }
                }
            }
            Rectangle {
                width: (parent.width - 10) / 2; height: 40; radius: 8
                color: withdrawArea.containsMouse ? "#D64158" : "#F87171"
                Text { anchors.centerIn: parent; text: "Withdraw"; color: "#2A0A0E"; font.pixelSize: 13; font.bold: true }
                MouseArea {
                    id: withdrawArea
                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        var ok = engine.withdrawCollateral(parseFloat(amountField.text || "0"))
                        statusText.text = ok ? "Withdrawn." : engine.lastError
                        statusText.color = ok ? "#34D399" : "#F87171"
                    }
                }
            }
        }

        Text { id: statusText; width: parent.width; wrapMode: Text.WordWrap; font.pixelSize: 12 }
    }
}
