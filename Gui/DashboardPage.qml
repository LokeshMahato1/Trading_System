import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic 2.15

Item {
    id: dashboard

    TradeDialog { id: tradeDialog }
    OrdersDialog { id: ordersDialog }
    CollateralDialog { id: collateralDialog }
    PortfolioDialog { id: portfolioDialog }

    Row {
        anchors.fill: parent
        spacing: 0

        // ================= LEFT NAV RAIL =================
        Rectangle {
            width: 72; height: parent.height
            color: "#0F1723"
            border.color: "#1E2D40"

            Column {
                anchors.top: parent.top
                anchors.topMargin: 16
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 4

                Rectangle {
                    width: 48; height: 48; radius: 8
                    color: "#1A3A2A"
                    anchors.horizontalCenter: parent.horizontalCenter
                    Column {
                        anchors.centerIn: parent; spacing: 2
                        Text { text: "⊞"; color: "#F0B429"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "Dashboard"; color: "#94A3B8"; font.pixelSize: 8; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }

                Repeater {
                    model: [
                        {icon:"≡", label:"Orders"},
                        {icon:"◎", label:"Portfolio"},
                        {icon:"$", label:"Collateral"},
                        {icon:"⚡", label:"Trade"}
                    ]
                    delegate: Rectangle {
                        width: 48; height: 52; radius: 8
                        color: sideMouseArea.containsMouse ? "#1A2236" : "transparent"
                        anchors.horizontalCenter: parent.horizontalCenter
                        Column {
                            anchors.centerIn: parent; spacing: 2
                            Text { text: modelData.icon; color: "#64748B"; font.pixelSize: 16; anchors.horizontalCenter: parent.horizontalCenter }
                            Text { text: modelData.label; color: "#64748B"; font.pixelSize: 8; anchors.horizontalCenter: parent.horizontalCenter }
                        }
                        MouseArea {
                            id: sideMouseArea
                            anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (modelData.label === "Orders") ordersDialog.open()
                                else if (modelData.label === "Portfolio") portfolioDialog.open()
                                else if (modelData.label === "Collateral") collateralDialog.open()
                                else if (modelData.label === "Trade") { tradeDialog.prefillSymbol = ""; tradeDialog.open() }
                            }
                        }
                    }
                }
            }
        }

        // ================= CENTER COLUMN =================
        Column {
            width: parent.width - 72 - 300
            height: parent.height
            spacing: 0

            // --- top bar ---
            Rectangle {
                width: parent.width; height: 56
                color: "#0F1723"; border.color: "#1E2D40"

                Row {
                    anchors.left: parent.left; anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter; spacing: 16

                    Row {
                        spacing: 10
                        Rectangle {
                            width: 36; height: 36; radius: 8; color: "#F0B429"
                            Text { anchors.centerIn: parent; text: "AB"; color: "#000"; font.pixelSize: 13; font.bold: true }
                        }
                        Column {
                            anchors.verticalCenter: parent.verticalCenter; spacing: 1
                            Text { text: "Artha Bazaar"; color: "#E2E8F0"; font.pixelSize: 15; font.bold: true }
                            Text { text: "Nepal Stock Exchange"; color: "#64748B"; font.pixelSize: 10 }
                        }
                    }
                }

                Row {
                    anchors.right: parent.right; anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter; spacing: 12

                    Column {
                        anchors.verticalCenter: parent.verticalCenter; spacing: 2
                        Row {
                            spacing: 6
                            Rectangle { width: 8; height: 8; radius: 4; color: engine.marketOpen ? "#34D399" : "#F87171"; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: engine.marketOpen ? "Market Open" : "Market Closed"; color: engine.marketOpen ? "#34D399" : "#F87171"; font.pixelSize: 12; font.bold: true }
                        }
                        Text { text: engine.currentDate + "  |  " + engine.currentTime; color: "#64748B"; font.pixelSize: 10 }
                    }

                    Rectangle {
                        width: 34; height: 34; radius: 17; color: "#F0B429"
                        Text { anchors.centerIn: parent; text: engine.displayName.length > 0 ? engine.displayName.substring(0,2).toUpperCase() : "?"; color: "#000"; font.pixelSize: 12; font.bold: true }
                        MouseArea {
                            anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: profileMenu.open()
                        }
                        Menu {
                            id: profileMenu
                            y: parent.height + 6
                            background: Rectangle { color: "#1A2236"; border.color: "#2A3348"; border.width: 1; radius: 8 }
                            MenuItem {
                                text: engine.username
                                enabled: false
                                contentItem: Text { text: parent.text; color: "#64748B"; font.pixelSize: 11; leftPadding: 12 }
                                background: Rectangle { color: "transparent" }
                            }
                            MenuItem {
                                text: "Logout"
                                contentItem: Text { text: parent.text; color: "#F87171"; font.pixelSize: 13; leftPadding: 12 }
                                background: Rectangle { color: parent.hovered ? "#2A3348" : "transparent" }
                                onTriggered: engine.logout()
                            }
                        }
                    }
                }
            }

            // --- ticker bar ---
            Rectangle {
                width: parent.width; height: 44
                color: "#0F1723"; border.color: "#1E2D40"

                Flickable {
                    anchors.left: parent.left; anchors.leftMargin: 16; anchors.right: parent.right; anchors.rightMargin: 100
                    anchors.verticalCenter: parent.verticalCenter
                    height: parent.height
                    contentWidth: tickerRow.width
                    clip: true
                    Row {
                        id: tickerRow
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 28
                        Repeater {
                            model: engine.stocks
                            delegate: Column {
                                spacing: 1
                                Text { text: modelData.symbol; color: "#64748B"; font.pixelSize: 10 }
                                Text { text: modelData.price.toFixed(2); color: "#E2E8F0"; font.pixelSize: 13; font.bold: true }
                                Text {
                                    text: (modelData.up ? "+" : "") + modelData.change.toFixed(2) + " (" + modelData.percentChange.toFixed(2) + "%)"
                                    color: modelData.up ? "#34D399" : "#F87171"; font.pixelSize: 10
                                }
                            }
                        }
                    }
                }
                Text {
                    anchors.right: parent.right; anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Live · NEPSE"; color: "#4A5568"; font.pixelSize: 11
                }
            }

            // --- main content ---
            Rectangle {
                width: parent.width
                height: parent.height - 56 - 44
                color: "#0D1520"

                Column {
                    anchors.fill: parent; anchors.margins: 12; spacing: 10

                    // --- index chart ---
                    Rectangle {
                        width: parent.width; height: 300
                        color: "#0F1723"; radius: 10; border.color: "#1E2D40"

                        Column {
                            anchors.fill: parent; anchors.margins: 16; spacing: 10

                            Row {
                                spacing: 8
                                Text { text: "Market Index"; color: "#E2E8F0"; font.pixelSize: 15; font.bold: true; anchors.verticalCenter: parent.verticalCenter }
                                Text {
                                    text: engine.marketIndexValue.toFixed(2) + "  " + (engine.marketIndexChangePercent >= 0 ? "+" : "") + engine.marketIndexChangePercent.toFixed(2) + "%"
                                    color: engine.marketIndexChangePercent >= 0 ? "#34D399" : "#F87171"
                                    font.pixelSize: 13; font.bold: true; anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            Canvas {
                                id: indexCanvas
                                width: parent.width; height: 210
                                property var pts: engine.marketIndexHistory
                                Connections {
                                    target: engine
                                    function onMarketChanged() { indexCanvas.pts = engine.marketIndexHistory; indexCanvas.requestPaint() }
                                }
                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.clearRect(0, 0, width, height)
                                    ctx.strokeStyle = "#1E2D40"; ctx.lineWidth = 0.5
                                    for (var g = 0; g < 5; g++) {
                                        ctx.beginPath(); ctx.moveTo(0, g * height / 4); ctx.lineTo(width, g * height / 4); ctx.stroke()
                                    }
                                    if (pts.length < 2) return
                                    var minY = pts[0].y, maxY = pts[0].y
                                    for (var i = 0; i < pts.length; i++) {
                                        minY = Math.min(minY, pts[i].y); maxY = Math.max(maxY, pts[i].y)
                                    }
                                    if (maxY - minY < 0.01) { maxY += 1; minY -= 1 }
                                    var chartH = height - 16
                                    function xAt(i) { return (pts[i].x / (pts.length - 1)) * width }
                                    function yAt(v) { return chartH - ((v - minY) / (maxY - minY)) * chartH }

                                    ctx.beginPath()
                                    ctx.moveTo(xAt(0), yAt(pts[0].y))
                                    for (var j = 1; j < pts.length; j++) ctx.lineTo(xAt(j), yAt(pts[j].y))
                                    ctx.strokeStyle = "#F0B429"; ctx.lineWidth = 2; ctx.stroke()
                                    ctx.lineTo(width, height); ctx.lineTo(0, height); ctx.closePath()
                                    ctx.fillStyle = Qt.rgba(0.94, 0.71, 0.16, 0.08); ctx.fill()
                                }
                            }
                        }
                    }

                    // --- stats row ---
                    Row {
                        width: parent.width; spacing: 8
                        Repeater {
                            model: [
                                {label:"Advances",     val: engine.advancesCount,                                color:"#34D399"},
                                {label:"Declines",      val: engine.declinesCount,                                color:"#F87171"},
                                {label:"Listed Symbols",val: engine.stocks.length,                                color:"#E2E8F0"},
                                {label:"Volume",        val: engine.totalVolume.toLocaleString(),                 color:"#E2E8F0"},
                                {label:"Turnover",      val: "Rs " + (engine.totalTurnover/1000).toFixed(1) + "K", color:"#E2E8F0"},
                                {label:"Filled Trades", val: engine.filledTradeCount,                             color:"#E2E8F0"}
                            ]
                            delegate: Rectangle {
                                width: (parent.width - 40) / 6; height: 52
                                color: "#0F1723"; radius: 8; border.color: "#1E2D40"
                                Column {
                                    anchors.centerIn: parent; spacing: 4
                                    Text { text: modelData.label; color: "#64748B"; font.pixelSize: 10; anchors.horizontalCenter: parent.horizontalCenter }
                                    Text { text: String(modelData.val); color: modelData.color; font.pixelSize: 13; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                                }
                            }
                        }
                    }

                    // --- gainers / losers ---
                    Row {
                        width: parent.width; spacing: 10

                        Rectangle {
                            width: (parent.width - 10) / 2; height: 240
                            color: "#0F1723"; radius: 10; border.color: "#1E2D40"
                            Column {
                                anchors.fill: parent; anchors.margins: 12; spacing: 7
                                Text { text: "Top Gainers"; color: "#34D399"; font.pixelSize: 13; font.bold: true }
                                Row {
                                    width: parent.width
                                    Text { text: "Symbol";  color: "#4A5568"; font.pixelSize: 11; width: parent.width*0.35 }
                                    Text { text: "LTP";     color: "#4A5568"; font.pixelSize: 11; width: parent.width*0.2;  horizontalAlignment: Text.AlignRight }
                                    Text { text: "% Change";color: "#4A5568"; font.pixelSize: 11; width: parent.width*0.25; horizontalAlignment: Text.AlignRight }
                                    Text { text: "Volume";  color: "#4A5568"; font.pixelSize: 11; width: parent.width*0.2;  horizontalAlignment: Text.AlignRight }
                                }
                                Repeater {
                                    model: engine.topGainers
                                    delegate: Row {
                                        width: parent.width
                                        Row {
                                            width: parent.width*0.35; spacing: 5
                                            Rectangle {
                                                width: 22; height: 22; radius: 4; color: "#1A3A2A"
                                                Text { anchors.centerIn: parent; text: modelData.symbol.charAt(0); color: "#34D399"; font.pixelSize: 10; font.bold: true }
                                            }
                                            Column {
                                                anchors.verticalCenter: parent.verticalCenter; spacing: 1
                                                Text { text: modelData.symbol;  color: "#E2E8F0"; font.pixelSize: 11; font.bold: true }
                                                Text { text: modelData.companyName; color: "#4A5568"; font.pixelSize: 9; elide: Text.ElideRight; width: 100 }
                                            }
                                        }
                                        Text { text: modelData.price.toFixed(2); color: "#E2E8F0"; font.pixelSize: 11; width: parent.width*0.2;  horizontalAlignment: Text.AlignRight; anchors.verticalCenter: parent.verticalCenter }
                                        Text { text: (modelData.percentChange>=0?"+":"") + modelData.percentChange.toFixed(2) + "%"; color: "#34D399"; font.pixelSize: 11; width: parent.width*0.25; horizontalAlignment: Text.AlignRight; anchors.verticalCenter: parent.verticalCenter }
                                        Text { text: modelData.volume; color: "#64748B"; font.pixelSize: 11; width: parent.width*0.2;  horizontalAlignment: Text.AlignRight; anchors.verticalCenter: parent.verticalCenter }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            width: (parent.width - 10) / 2; height: 240
                            color: "#0F1723"; radius: 10; border.color: "#1E2D40"
                            Column {
                                anchors.fill: parent; anchors.margins: 12; spacing: 7
                                Text { text: "Top Losers"; color: "#F87171"; font.pixelSize: 13; font.bold: true }
                                Row {
                                    width: parent.width
                                    Text { text: "Symbol";  color: "#4A5568"; font.pixelSize: 11; width: parent.width*0.35 }
                                    Text { text: "LTP";     color: "#4A5568"; font.pixelSize: 11; width: parent.width*0.2;  horizontalAlignment: Text.AlignRight }
                                    Text { text: "% Change";color: "#4A5568"; font.pixelSize: 11; width: parent.width*0.25; horizontalAlignment: Text.AlignRight }
                                    Text { text: "Volume";  color: "#4A5568"; font.pixelSize: 11; width: parent.width*0.2;  horizontalAlignment: Text.AlignRight }
                                }
                                Repeater {
                                    model: engine.topLosers
                                    delegate: Row {
                                        width: parent.width
                                        Row {
                                            width: parent.width*0.35; spacing: 5
                                            Rectangle {
                                                width: 22; height: 22; radius: 4; color: "#3D1515"
                                                Text { anchors.centerIn: parent; text: modelData.symbol.charAt(0); color: "#F87171"; font.pixelSize: 10; font.bold: true }
                                            }
                                            Column {
                                                anchors.verticalCenter: parent.verticalCenter; spacing: 1
                                                Text { text: modelData.symbol;  color: "#E2E8F0"; font.pixelSize: 11; font.bold: true }
                                                Text { text: modelData.companyName; color: "#4A5568"; font.pixelSize: 9; elide: Text.ElideRight; width: 100 }
                                            }
                                        }
                                        Text { text: modelData.price.toFixed(2); color: "#E2E8F0"; font.pixelSize: 11; width: parent.width*0.2;  horizontalAlignment: Text.AlignRight; anchors.verticalCenter: parent.verticalCenter }
                                        Text { text: modelData.percentChange.toFixed(2) + "%"; color: "#F87171"; font.pixelSize: 11; width: parent.width*0.25; horizontalAlignment: Text.AlignRight; anchors.verticalCenter: parent.verticalCenter }
                                        Text { text: modelData.volume; color: "#64748B"; font.pixelSize: 11; width: parent.width*0.2;  horizontalAlignment: Text.AlignRight; anchors.verticalCenter: parent.verticalCenter }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // ================= RIGHT SIDEBAR =================
        Rectangle {
            width: 300; height: parent.height
            color: "#0F1723"; border.color: "#1E2D40"

            Column {
                anchors.fill: parent; anchors.margins: 14; spacing: 10

                // --- total portfolio ---
                Rectangle {
                    width: parent.width; height: 90
                    color: "#0D1520"; radius: 10; border.color: "#1E2D40"
                    Column {
                        anchors.fill: parent; anchors.margins: 14; spacing: 4
                        Text { text: "Total Portfolio"; color: "#94A3B8"; font.pixelSize: 12 }
                        Text { text: "Rs " + engine.portfolioValue.toFixed(2); color: "#E2E8F0"; font.pixelSize: 22; font.bold: true }
                        Text {
                            text: (engine.portfolioPL>=0?"+":"") + "Rs " + engine.portfolioPL.toFixed(2) + " (" + engine.portfolioPLPercent.toFixed(2) + "%) unrealized"
                            color: engine.portfolioPL>=0 ? "#34D399" : "#F87171"; font.pixelSize: 11
                        }
                    }
                }

                // --- holdings ---
                Rectangle {
                    width: parent.width; height: 155
                    color: "#0D1520"; radius: 10; border.color: "#1E2D40"
                    Column {
                        anchors.fill: parent; anchors.margins: 14; spacing: 7
                        Row {
                            width: parent.width
                            Text { text: "Holdings"; color: "#E2E8F0"; font.pixelSize: 13; font.bold: true }
                            Item { width: parent.width - 80; height: 1 }
                            Text {
                                text: "View all"; color: "#F0B429"; font.pixelSize: 11
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: portfolioDialog.open() }
                            }
                        }
                        Repeater {
                            model: engine.holdings.length > 4 ? engine.holdings.slice(0, 4) : engine.holdings
                            delegate: Row {
                                width: parent.width
                                Text { text: modelData.symbol; color: "#E2E8F0"; font.pixelSize: 12; font.bold: true; width: 54 }
                                Text { text: "· " + modelData.quantity + " shares"; color: "#64748B"; font.pixelSize: 11; width: parent.width - 108 }
                                Text {
                                    text: (modelData.changePercent>=0?"+":"") + modelData.changePercent.toFixed(1) + "%"
                                    color: modelData.up ? "#34D399" : "#F87171"; font.pixelSize: 12; font.bold: true
                                    horizontalAlignment: Text.AlignRight; width: 54
                                }
                            }
                        }
                        Text {
                            visible: engine.holdings.length === 0
                            text: "No holdings yet."; color: "#4A5568"; font.pixelSize: 11
                        }
                    }
                }

                // --- market breadth ---
                Rectangle {
                    width: parent.width; height: 120
                    color: "#0D1520"; radius: 10; border.color: "#1E2D40"
                    Column {
                        anchors.fill: parent; anchors.margins: 14; spacing: 6
                        Text { text: "Market Breadth"; color: "#E2E8F0"; font.pixelSize: 13; font.bold: true }
                        Rectangle {
                            width: parent.width; height: 6; radius: 3
                            color: "#2A3348"
                            property real advPct: (engine.advancesCount + engine.declinesCount) > 0 ? engine.advancesCount / (engine.advancesCount + engine.declinesCount) : 0.5
                            Rectangle { width: parent.width*parent.advPct; height: 6; radius: 3; color: "#34D399" }
                            Rectangle { width: parent.width*(1-parent.advPct); height: 6; radius: 3; color: "#F87171"; anchors.right: parent.right }
                        }
                        Row {
                            width: parent.width
                            Text { text: engine.advancesCount + " advancing"; color: "#34D399"; font.pixelSize: 11; width: parent.width/2 }
                            Text { text: engine.declinesCount + " declining"; color: "#F87171"; font.pixelSize: 11; horizontalAlignment: Text.AlignRight; width: parent.width/2 }
                        }
                        Row {
                            width: parent.width
                            spacing: 6
                            Text { text: "Open orders:"; color: "#64748B"; font.pixelSize: 11 }
                            Text { text: engine.openOrderCount; color: "#E2E8F0"; font.pixelSize: 11; font.bold: true }
                        }
                    }
                }

                // --- quick actions ---
                Column {
                    width: parent.width; spacing: 8
                    Text { text: "Quick Actions"; color: "#E2E8F0"; font.pixelSize: 13; font.bold: true }
                    Row {
                        width: parent.width; spacing: 8
                        Rectangle {
                            width: (parent.width-8)/2; height: 40; radius: 8
                            color: buyM.containsMouse ? "#C49020" : "#F0B429"
                            Text { anchors.centerIn: parent; text: "Buy / Sell"; color: "#000"; font.pixelSize: 12; font.bold: true }
                            MouseArea { id: buyM; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: { tradeDialog.prefillSymbol = ""; tradeDialog.open() } }
                        }
                        Rectangle {
                            width: (parent.width-8)/2; height: 40; radius: 8
                            color: ordM.containsMouse ? "#1A2236" : "#1E2D40"
                            Text { anchors.centerIn: parent; text: "My Orders"; color: "#94A3B8"; font.pixelSize: 12 }
                            MouseArea { id: ordM; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: ordersDialog.open() }
                        }
                    }
                    Row {
                        width: parent.width; spacing: 8
                        Rectangle {
                            width: (parent.width-8)/2; height: 40; radius: 8
                            color: portM.containsMouse ? "#1A2236" : "#1E2D40"
                            Text { anchors.centerIn: parent; text: "Portfolio"; color: "#94A3B8"; font.pixelSize: 12 }
                            MouseArea { id: portM; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: portfolioDialog.open() }
                        }
                        Rectangle {
                            width: (parent.width-8)/2; height: 40; radius: 8
                            color: fundM.containsMouse ? "#1A2236" : "#1E2D40"
                            Text { anchors.centerIn: parent; text: "Transfer"; color: "#94A3B8"; font.pixelSize: 12 }
                            MouseArea { id: fundM; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: collateralDialog.open() }
                        }
                    }
                }

                // --- market summary ---
                Rectangle {
                    width: parent.width; height: 115
                    color: "#0D1520"; radius: 10; border.color: "#1E2D40"
                    Column {
                        anchors.fill: parent; anchors.margins: 14; spacing: 6
                        Text { text: "Market Summary"; color: "#E2E8F0"; font.pixelSize: 13; font.bold: true }
                        Row {
                            width: parent.width
                            Text { text: "Total Turnover"; color: "#64748B"; font.pixelSize: 11; width: parent.width - 70 }
                            Text { text: "Rs " + (engine.totalTurnover/1000).toFixed(1) + "K"; color: "#E2E8F0"; font.pixelSize: 11; font.bold: true; horizontalAlignment: Text.AlignRight; width: 70 }
                        }
                        Row {
                            width: parent.width
                            Text { text: "Total Volume"; color: "#64748B"; font.pixelSize: 11; width: parent.width - 70 }
                            Text { text: engine.totalVolume.toLocaleString(); color: "#E2E8F0"; font.pixelSize: 11; font.bold: true; horizontalAlignment: Text.AlignRight; width: 70 }
                        }
                        Row {
                            width: parent.width
                            Text { text: "Filled Trades"; color: "#64748B"; font.pixelSize: 11; width: parent.width - 70 }
                            Text { text: engine.filledTradeCount; color: "#E2E8F0"; font.pixelSize: 11; font.bold: true; horizontalAlignment: Text.AlignRight; width: 70 }
                        }
                        Row {
                            width: parent.width
                            Text { text: "Advances / Declines"; color: "#64748B"; font.pixelSize: 11; width: parent.width - 70 }
                            Text { text: engine.advancesCount + " / " + engine.declinesCount; color: "#E2E8F0"; font.pixelSize: 11; font.bold: true; horizontalAlignment: Text.AlignRight; width: 70 }
                        }
                    }
                }
            }
        }
    }
}
