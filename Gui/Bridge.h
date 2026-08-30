#ifndef BRIDGE_H
#define BRIDGE_H

// Bridge is the single QObject exposed to QML (as the context property
// "engine") that connects the Artha Bazaar QML front-end to the existing
// C++ trading engine (Database, Market, Admin, Client, OrderBook,
// MatchMaking, TradeExecution). None of the Engine/ sources are modified;
// this class drives their existing public APIs, the same way the console
// TradingSystem and the (now removed) Qt Widgets front-end did.
//
// QML binds directly to this object's Q_PROPERTYs (which is why almost
// everything here is a property with a NOTIFY signal) and calls its
// Q_INVOKABLE methods for actions (login, placeBuyOrder, etc).

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <memory>

#include "../Engine/database/db.hpp"
#include "../Engine/market/market.h"
#include "../Engine/orderbook/orderBook.hpp"
#include "../Engine/Matchmaking/matchmaking.h"
#include "../Engine/execution/tradeExecution.h"
#include "../Engine/users/user.h"

class Bridge : public QObject
{
    Q_OBJECT

    // --- session ---------------------------------------------------------
    Q_PROPERTY(bool loggedIn READ loggedIn NOTIFY sessionChanged)
    Q_PROPERTY(bool isAdmin READ isAdmin NOTIFY sessionChanged)
    Q_PROPERTY(QString username READ username NOTIFY sessionChanged)
    Q_PROPERTY(QString displayName READ displayName NOTIFY sessionChanged)
    Q_PROPERTY(bool hasUsers READ hasUsers NOTIFY sessionChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

    // --- market ---------------------------------------------------------
    Q_PROPERTY(bool marketOpen READ marketOpen NOTIFY marketChanged)
    Q_PROPERTY(QString currentTime READ currentTime NOTIFY clockChanged)
    Q_PROPERTY(QString currentDate READ currentDate NOTIFY clockChanged)
    Q_PROPERTY(QVariantList stocks READ stocks NOTIFY marketChanged)
    Q_PROPERTY(QVariantList topGainers READ topGainers NOTIFY marketChanged)
    Q_PROPERTY(QVariantList topLosers READ topLosers NOTIFY marketChanged)
    Q_PROPERTY(double marketIndexValue READ marketIndexValue NOTIFY marketChanged)
    Q_PROPERTY(double marketIndexChangePercent READ marketIndexChangePercent NOTIFY marketChanged)
    Q_PROPERTY(QVariantList marketIndexHistory READ marketIndexHistory NOTIFY marketChanged)
    Q_PROPERTY(double totalVolume READ totalVolume NOTIFY marketChanged)
    Q_PROPERTY(double totalTurnover READ totalTurnover NOTIFY marketChanged)
    Q_PROPERTY(int advancesCount READ advancesCount NOTIFY marketChanged)
    Q_PROPERTY(int declinesCount READ declinesCount NOTIFY marketChanged)
    Q_PROPERTY(int filledTradeCount READ filledTradeCount NOTIFY ordersChanged)

    // --- portfolio / collateral (client) ---------------------------------
    Q_PROPERTY(double portfolioValue READ portfolioValue NOTIFY portfolioChanged)
    Q_PROPERTY(double portfolioPL READ portfolioPL NOTIFY portfolioChanged)
    Q_PROPERTY(double portfolioPLPercent READ portfolioPLPercent NOTIFY portfolioChanged)
    Q_PROPERTY(double collateralAvailable READ collateralAvailable NOTIFY portfolioChanged)
    Q_PROPERTY(double collateralBlocked READ collateralBlocked NOTIFY portfolioChanged)
    Q_PROPERTY(double collateralTotal READ collateralTotal NOTIFY portfolioChanged)
    Q_PROPERTY(QVariantList holdings READ holdings NOTIFY portfolioChanged)
    Q_PROPERTY(QVariantList myOrders READ myOrders NOTIFY ordersChanged)
    Q_PROPERTY(int openOrderCount READ openOrderCount NOTIFY ordersChanged)

    // --- admin ---------------------------------------------------------
    Q_PROPERTY(QVariantList allUsers READ allUsers NOTIFY usersChanged)
    Q_PROPERTY(QVariantList allOrders READ allOrders NOTIFY ordersChanged)
    Q_PROPERTY(int userCount READ userCount NOTIFY usersChanged)
    Q_PROPERTY(int orderCount READ orderCount NOTIFY ordersChanged)

public:
    explicit Bridge(QObject* parent = nullptr);
    ~Bridge() override;

    Q_INVOKABLE bool initialize();

    // --- session ---------------------------------------------------------
    bool loggedIn() const;
    bool isAdmin() const;
    QString username() const;
    QString displayName() const;
    bool hasUsers();
    QString lastError() const;

    Q_INVOKABLE bool login(const QString& username, const QString& password);
    Q_INVOKABLE void logout();
    Q_INVOKABLE bool bootstrapAdmin(const QString& name, const QString& username, const QString& password);

    // --- market ---------------------------------------------------------
    bool marketOpen();
    QString currentTime();
    QString currentDate();
    QVariantList stocks();
    QVariantList topGainers();
    QVariantList topLosers();
    double marketIndexValue();
    double marketIndexChangePercent();
    QVariantList marketIndexHistory();
    double totalVolume();
    double totalTurnover();
    int advancesCount();
    int declinesCount();
    int filledTradeCount();
    Q_INVOKABLE void setMarketOpen(bool open);

    // --- portfolio / collateral --------------------------------------------
    double portfolioValue();
    double portfolioPL();
    double portfolioPLPercent();
    double collateralAvailable();
    double collateralBlocked();
    double collateralTotal();
    QVariantList holdings();
    QVariantList myOrders();
    int openOrderCount();

    Q_INVOKABLE bool placeBuyOrder(const QString& symbol, int quantity, double price);
    Q_INVOKABLE bool placeSellOrder(const QString& symbol, int quantity, double price);
    Q_INVOKABLE bool cancelOrder(int orderId);
    Q_INVOKABLE bool depositCollateral(double amount);
    Q_INVOKABLE bool withdrawCollateral(double amount);

    // --- admin ---------------------------------------------------------
    QVariantList allUsers();
    QVariantList allOrders();
    int userCount();
    int orderCount();

    Q_INVOKABLE bool addUser(const QString& name, const QString& username, const QString& password, const QString& role);
    Q_INVOKABLE bool removeUser(int userId);
    Q_INVOKABLE bool resetPassword(int userId, const QString& newPassword);

signals:
    void sessionChanged();
    void marketChanged();
    void clockChanged();
    void portfolioChanged();
    void ordersChanged();
    void usersChanged();
    void lastErrorChanged();

private slots:
    void tick();
    void tickClock();
    void tickMarketRefresh();

private:
    void setLastError(const QString& error);
    bool loadClientState(int clientId, Portfolio& portfolio, Collateral& collateral);
    void runMatchingEngine();
    void sampleIndexHistory();

    Database db;
    Market market;
    TradeExecution tradeExecution;
    MatchMaking matchmaking;
    OrderBook orderBook;

    std::unique_ptr<User> currentUser;
    QString m_lastError;
    QTimer m_timer;
    QTimer m_clockTimer;
    QTimer m_marketRefreshTimer;
    bool m_marketRefreshInProgress = false;
    QVector<double> m_indexHistory;
};

#endif // BRIDGE_H
