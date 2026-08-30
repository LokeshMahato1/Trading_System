import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic 2.15

Item {
    id: adminPage

    Popup {
        id: resetPasswordPopup
        width: 340; height: 190
        modal: true; focus: true; anchors.centerIn: parent
        property int userId: -1
        background: Rectangle { color: "#0F1723"; radius: 12; border.color: "#1E2D40" }
        Column {
            anchors.fill: parent; anchors.margins: 20; spacing: 10
            Text { text: "Reset password"; color: "#E2E8F0"; font.pixelSize: 14; font.bold: true }
            TextField {
                id: newPasswordField
                width: parent.width
                echoMode: TextInput.Password
                placeholderText: "New password"
                color: "#E2E8F0"; placeholderTextColor: "#4A5568"
                background: Rectangle { color: "#0D1520"; radius: 7; border.color: "#1E2D40" }
                leftPadding: 12
            }
            Rectangle {
                width: parent.width; height: 36; radius: 8
                color: "#F0B429"
                Text { anchors.centerIn: parent; text: "Reset"; color: "#000"; font.pixelSize: 12; font.bold: true }
                MouseArea {
                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        engine.resetPassword(resetPasswordPopup.userId, newPasswordField.text)
                        resetPasswordPopup.close()
                    }
                }
            }
        }
    }

    Column {
        anchors.fill: parent

        // --- top bar ---
        Rectangle {
            width: parent.width; height: 56
            color: "#0F1723"; border.color: "#1E2D40"

            Row {
                anchors.left: parent.left; anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter; spacing: 10
                Rectangle {
                    width: 36; height: 36; radius: 8; color: "#F0B429"
                    Text { anchors.centerIn: parent; text: "AB"; color: "#000"; font.pixelSize: 13; font.bold: true }
                }
                Column {
                    anchors.verticalCenter: parent.verticalCenter; spacing: 1
                    Text { text: "Artha Bazaar — Admin"; color: "#E2E8F0"; font.pixelSize: 15; font.bold: true }
                    Text { text: "Signed in as " + engine.username; color: "#64748B"; font.pixelSize: 10 }
                }
            }

            Row {
                anchors.right: parent.right; anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter; spacing: 12

                Rectangle {
                    width: 130; height: 30; radius: 6
                    color: engine.marketOpen ? "#1A3A2A" : "#3D1515"
                    Text {
                        anchors.centerIn: parent
                        text: engine.marketOpen ? "● Close Market" : "● Open Market"
                        color: engine.marketOpen ? "#34D399" : "#F87171"
                        font.pixelSize: 11; font.bold: true
                    }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: engine.setMarketOpen(!engine.marketOpen) }
                }

                Rectangle {
                    width: 80; height: 30; radius: 6
                    color: "#1E2D40"
                    Text { anchors.centerIn: parent; text: "Log out"; color: "#94A3B8"; font.pixelSize: 11 }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: engine.logout() }
                }
            }
        }

        Rectangle {
            width: parent.width
            height: parent.height - 56
            color: "#0D1520"

            Column {
                anchors.fill: parent; anchors.margins: 16; spacing: 14

                // --- stat cards ---
                Row {
                    width: parent.width; spacing: 10
                    Repeater {
                        model: [
                            {label:"Total Users",    val: engine.userCount},
                            {label:"Total Orders",   val: engine.orderCount},
                            {label:"Listed Symbols", val: engine.stocks.length},
                            {label:"Filled Trades",  val: engine.filledTradeCount}
                        ]
                        delegate: Rectangle {
                            width: (parent.width - 30) / 4; height: 78
                            color: "#0F1723"; radius: 10; border.color: "#1E2D40"
                            Column {
                                anchors.fill: parent; anchors.margins: 14; spacing: 6
                                Text { text: modelData.label.toUpperCase(); color: "#64748B"; font.pixelSize: 10; font.bold: true }
                                Text { text: String(modelData.val); color: "#E2E8F0"; font.pixelSize: 20; font.bold: true }
                            }
                        }
                    }
                }

                Row {
                    width: parent.width
                    height: parent.height - 78 - 14
                    spacing: 14

                    // --- users panel ---
                    Rectangle {
                        width: (parent.width - 14) / 2; height: parent.height
                        color: "#0F1723"; radius: 10; border.color: "#1E2D40"
                        Column {
                            anchors.fill: parent; anchors.margins: 16; spacing: 8
                            Text { text: "Users"; color: "#E2E8F0"; font.pixelSize: 14; font.bold: true }

                            Row {
                                width: parent.width
                                Text { text: "ID"; color: "#4A5568"; font.pixelSize: 10; width: parent.width*0.12 }
                                Text { text: "Name"; color: "#4A5568"; font.pixelSize: 10; width: parent.width*0.3 }
                                Text { text: "Username"; color: "#4A5568"; font.pixelSize: 10; width: parent.width*0.28 }
                                Text { text: "Role"; color: "#4A5568"; font.pixelSize: 10; width: parent.width*0.15 }
                            }
                            Rectangle { width: parent.width; height: 1; color: "#1E2D40" }

                            ListView {
                                width: parent.width
                                height: parent.height - 240
                                clip: true
                                model: engine.allUsers
                                delegate: Row {
                                    width: ListView.view.width; height: 30
                                    Text { text: modelData.id; color: "#94A3B8"; font.pixelSize: 11; width: parent.width*0.12; anchors.verticalCenter: parent.verticalCenter }
                                    Text { text: modelData.name; color: "#E2E8F0"; font.pixelSize: 11; width: parent.width*0.3; anchors.verticalCenter: parent.verticalCenter }
                                    Text { text: modelData.username; color: "#E2E8F0"; font.pixelSize: 11; width: parent.width*0.28; anchors.verticalCenter: parent.verticalCenter }
                                    Text {
                                        text: modelData.role; font.pixelSize: 10; font.bold: true; width: parent.width*0.15
                                        color: modelData.role === "ADMIN" ? "#F0B429" : "#5B8DEF"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                    Row {
                                        spacing: 4
                                        anchors.verticalCenter: parent.verticalCenter
                                        Rectangle {
                                            width: 20; height: 20; radius: 4; color: "#3D1515"
                                            Text { anchors.centerIn: parent; text: "✕"; color: "#F87171"; font.pixelSize: 10 }
                                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: engine.removeUser(modelData.id) }
                                        }
                                        Rectangle {
                                            width: 20; height: 20; radius: 4; color: "#1E2D40"
                                            Text { anchors.centerIn: parent; text: "⚷"; color: "#94A3B8"; font.pixelSize: 10 }
                                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { resetPasswordPopup.userId = modelData.id; newPasswordField.text = ""; resetPasswordPopup.open() } }
                                        }
                                    }
                                }
                            }

                            Rectangle { width: parent.width; height: 1; color: "#1E2D40" }

                            Text { text: "Add user"; color: "#94A3B8"; font.pixelSize: 11; font.bold: true }
                            Row {
                                width: parent.width; spacing: 6
                                TextField {
                                    id: newName; width: (parent.width - 12) / 4
                                    placeholderText: "Name"; color: "#E2E8F0"; placeholderTextColor: "#4A5568"
                                    background: Rectangle { color: "#0D1520"; radius: 6; border.color: "#1E2D40" }
                                    leftPadding: 8; font.pixelSize: 11
                                }
                                TextField {
                                    id: newUsername; width: (parent.width - 12) / 4
                                    placeholderText: "Username"; color: "#E2E8F0"; placeholderTextColor: "#4A5568"
                                    background: Rectangle { color: "#0D1520"; radius: 6; border.color: "#1E2D40" }
                                    leftPadding: 8; font.pixelSize: 11
                                }
                                TextField {
                                    id: newPassword; width: (parent.width - 12) / 4
                                    echoMode: TextInput.Password
                                    placeholderText: "Password"; color: "#E2E8F0"; placeholderTextColor: "#4A5568"
                                    background: Rectangle { color: "#0D1520"; radius: 6; border.color: "#1E2D40" }
                                    leftPadding: 8; font.pixelSize: 11
                                }
                                ComboBox {
                                    id: newRole; width: (parent.width - 12) / 4
                                    model: ["CLIENT", "ADMIN"]
                                    font.pixelSize: 11
                                }
                            }
                            Rectangle {
                                width: parent.width; height: 32; radius: 7
                                color: "#F0B429"
                                Text { anchors.centerIn: parent; text: "Add user"; color: "#000"; font.pixelSize: 11; font.bold: true }
                                MouseArea {
                                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (engine.addUser(newName.text, newUsername.text, newPassword.text, newRole.currentText)) {
                                            newName.text = ""; newUsername.text = ""; newPassword.text = ""
                                        }
                                    }
                                }
                            }
                            Text {
                                width: parent.width
                                text: engine.lastError; color: "#F87171"; font.pixelSize: 10; wrapMode: Text.WordWrap
                                visible: engine.lastError.length > 0
                            }
                        }
                    }

                    // --- orders panel ---
                    Rectangle {
                        width: (parent.width - 14) / 2; height: parent.height
                        color: "#0F1723"; radius: 10; border.color: "#1E2D40"
                        Column {
                            anchors.fill: parent; anchors.margins: 16; spacing: 8
                            Text { text: "All Orders"; color: "#E2E8F0"; font.pixelSize: 14; font.bold: true }
                            Row {
                                width: parent.width
                                Text { text: "ID"; color: "#4A5568"; font.pixelSize: 10; width: parent.width*0.1 }
                                Text { text: "User"; color: "#4A5568"; font.pixelSize: 10; width: parent.width*0.12 }
                                Text { text: "Symbol"; color: "#4A5568"; font.pixelSize: 10; width: parent.width*0.18 }
                                Text { text: "Type"; color: "#4A5568"; font.pixelSize: 10; width: parent.width*0.14 }
                                Text { text: "Qty"; color: "#4A5568"; font.pixelSize: 10; width: parent.width*0.18 }
                                Text { text: "Status"; color: "#4A5568"; font.pixelSize: 10; width: parent.width*0.2 }
                            }
                            Rectangle { width: parent.width; height: 1; color: "#1E2D40" }
                            ListView {
                                width: parent.width
                                height: parent.height - 60
                                clip: true
                                model: engine.allOrders
                                delegate: Row {
                                    width: ListView.view.width; height: 28
                                    Text { text: modelData.orderId; color: "#94A3B8"; font.pixelSize: 10; width: parent.width*0.1; anchors.verticalCenter: parent.verticalCenter }
                                    Text { text: modelData.userId; color: "#94A3B8"; font.pixelSize: 10; width: parent.width*0.12; anchors.verticalCenter: parent.verticalCenter }
                                    Text { text: modelData.symbol; color: "#E2E8F0"; font.pixelSize: 10; width: parent.width*0.18; anchors.verticalCenter: parent.verticalCenter }
                                    Text {
                                        text: modelData.type; font.pixelSize: 10; font.bold: true; width: parent.width*0.14
                                        color: modelData.type === "BUY" ? "#34D399" : "#F87171"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                    Text { text: modelData.remaining + "/" + modelData.quantity; color: "#94A3B8"; font.pixelSize: 10; width: parent.width*0.18; anchors.verticalCenter: parent.verticalCenter }
                                    Text {
                                        text: modelData.status; font.pixelSize: 9; font.bold: true; width: parent.width*0.2
                                        color: modelData.status === "FILLED" ? "#34D399"
                                               : (modelData.status === "FAILED" || modelData.status === "CANCELLED") ? "#F87171"
                                               : (modelData.status === "PARTIALLY_FILLED" ? "#F0B429" : "#5B8DEF")
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
