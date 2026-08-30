import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic 2.15

Rectangle {
    anchors.fill: parent
    color: "#0D1520"

    Rectangle {
        anchors.centerIn: parent
        width: 400
        height: bootstrapVisible ? 620 : 460
        radius: 14
        color: "#0F1723"
        border.color: "#1E2D40"

        property bool bootstrapVisible: !engine.hasUsers

        Column {
            anchors.fill: parent
            anchors.margins: 28
            spacing: 14

            Image {
                source: "loginLogo.png"
                fillMode: Image.PreserveAspectFit
                width: parent.width
                height: 130
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: "Nepal Stock Exchange · Trading Simulator"
                color: "#64748B"
                font.pixelSize: 12
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item { width: 1; height: 6 }

            Text { text: "Username"; color: "#94A3B8"; font.pixelSize: 11 }
            TextField {
                id: usernameField
                width: parent.width
                placeholderText: "e.g. lokesh01"
                color: "#E2E8F0"
                placeholderTextColor: "#4A5568"
                background: Rectangle { color: "#0D1520"; radius: 7; border.color: usernameField.activeFocus ? "#F0B429" : "#1E2D40" }
                leftPadding: 12
                onAccepted: passwordField.forceActiveFocus()
            }

            Text { text: "Password"; color: "#94A3B8"; font.pixelSize: 11 }
            TextField {
                id: passwordField
                width: parent.width
                echoMode: TextInput.Password
                placeholderText: "••••••••"
                color: "#E2E8F0"
                placeholderTextColor: "#4A5568"
                background: Rectangle { color: "#0D1520"; radius: 7; border.color: passwordField.activeFocus ? "#F0B429" : "#1E2D40" }
                leftPadding: 12
                onAccepted: loginButton.clicked()
            }

            Rectangle {
                id: loginButton
                width: parent.width; height: 42; radius: 8
                color: loginArea.containsMouse ? "#C49020" : "#F0B429"
                Text { anchors.centerIn: parent; text: "Log in"; color: "#000"; font.pixelSize: 13; font.bold: true }
                MouseArea {
                    id: loginArea
                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: parent.clicked()
                }
                signal clicked()
                onClicked: engine.login(usernameField.text, passwordField.text)
            }

            Text {
                width: parent.width
                text: engine.lastError
                color: "#F87171"
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                visible: engine.lastError.length > 0
            }

            // --- first-run bootstrap ---------------------------------------
            Rectangle {
                width: parent.width; height: 1; color: "#1E2D40"
                visible: parent.parent.bootstrapVisible
            }

            Column {
                width: parent.width
                spacing: 10
                visible: parent.parent.bootstrapVisible

                Text { text: "First run — create the administrator account"; color: "#F0B429"; font.pixelSize: 12; font.bold: true }

                TextField {
                    id: bootName
                    width: parent.width
                    placeholderText: "Name"
                    color: "#E2E8F0"; placeholderTextColor: "#4A5568"
                    background: Rectangle { color: "#0D1520"; radius: 7; border.color: "#1E2D40" }
                    leftPadding: 12
                }
                TextField {
                    id: bootUsername
                    width: parent.width
                    placeholderText: "Username"
                    color: "#E2E8F0"; placeholderTextColor: "#4A5568"
                    background: Rectangle { color: "#0D1520"; radius: 7; border.color: "#1E2D40" }
                    leftPadding: 12
                }
                TextField {
                    id: bootPassword
                    width: parent.width
                    echoMode: TextInput.Password
                    placeholderText: "Password"
                    color: "#E2E8F0"; placeholderTextColor: "#4A5568"
                    background: Rectangle { color: "#0D1520"; radius: 7; border.color: "#1E2D40" }
                    leftPadding: 12
                }

                Rectangle {
                    width: parent.width; height: 38; radius: 8
                    color: bootArea.containsMouse ? "#1A2236" : "#1E2D40"
                    Text { anchors.centerIn: parent; text: "Create admin account"; color: "#94A3B8"; font.pixelSize: 12 }
                    MouseArea {
                        id: bootArea
                        anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                        onClicked: engine.bootstrapAdmin(bootName.text, bootUsername.text, bootPassword.text)
                    }
                }
            }
        }
    }
}
