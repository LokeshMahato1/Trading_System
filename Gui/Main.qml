import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 1280
    height: 800
    visible: true
    title: "Artha Bazaar"
    color: "#151C28"

    Loader {
        id: pageLoader
        anchors.fill: parent
        sourceComponent: !engine.loggedIn ? loginComponent
                          : (engine.isAdmin ? adminComponent : dashboardComponent)
    }

    Component { id: loginComponent; LoginPage {} }
    Component { id: dashboardComponent; DashboardPage {} }
    Component { id: adminComponent; AdminPage {} }
}
