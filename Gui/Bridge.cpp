#include "Bridge.h"

#include "../Engine/users/admin.h"
#include "../Engine/users/client.h"

#include <QDateTime>
#include <QVariantMap>
#include <algorithm>

namespace {

QString orderTypeToString(OrderType type)
{
    return type == OrderType::BUY ? QStringLiteral("BUY") : QStringLiteral("SELL");
}

QString orderStatusToString(OrderStatus status)
{
    switch (status) {
        case OrderStatus::PENDING: return QStringLiteral("PENDING");
        case OrderStatus::PARTIALLY_FILLED: return QStringLiteral("PARTIALLY_FILLED");
        case OrderStatus::FILLED: return QStringLiteral("FILLED");
        case OrderStatus::FAILED: return QStringLiteral("FAILED");
        case OrderStatus::CANCELLED: return QStringLiteral("CANCELLED");
    }
    return QStringLiteral("UNKNOWN");
}

QVariantMap orderToMap(const Order& order)
{
    QVariantMap m;
    m["orderId"] = order.getOrderId();
    m["userId"] = order.getUserId();
    m["symbol"] = QString::fromStdString(order.getStockSymbol());
    m["quantity"] = order.getQuantity();
    m["remaining"] = order.getRemainingQuantity();
    m["price"] = order.getPrice();
    m["type"] = orderTypeToString(order.getType());
    m["status"] = orderStatusToString(order.getStatus());
    m["isOpen"] = (order.getStatus() == OrderStatus::PENDING || order.getStatus() == OrderStatus::PARTIALLY_FILLED);
    return m;
}

} // namespace

Bridge::Bridge(QObject* parent)
    : QObject(parent),
      db(),
      market(),
      tradeExecution(db),
      matchmaking()
{
    connect(&m_timer, &QTimer::timeout, this, &Bridge::tick);
    connect(&m_clockTimer, &QTimer::timeout, this, &Bridge::tickClock);
    connect(&m_marketRefreshTimer, &QTimer::timeout, this, &Bridge::tickMarketRefresh);
}

Bridge::~Bridge() = default;

bool Bridge::initialize()
{
    if (!db.createTables()) {
        setLastError(QStringLiteral("Failed to create/open database."));
        return false;
    }

    if (!market.loadStocks()) {
        setLastError(QStringLiteral(
            "Could not load live stock data (check API_KEY in .env). Using any cached data available."));
    }

    m_timer.start(3000);
    m_clockTimer.start(1000);
    // Live market-data refresh does a blocking HTTP request per symbol, so
    // it gets its own slow, non-overlapping timer instead of running on
    // every fast tick - otherwise the UI thread freezes every few seconds
    // for as long as those requests take (looks like a hang/crash).
    m_marketRefreshTimer.start(60000);
    tickMarketRefresh();
    tick();
    return true;
}

void Bridge::tick()
{
    // Fast, local-only work: match pending orders and resample the
    // (in-memory) composite index. No network I/O happens here.
    runMatchingEngine();
    sampleIndexHistory();
    emit marketChanged();
    if (currentUser) {
        emit portfolioChanged();
        emit ordersChanged();
        if (isAdmin()) {
            emit usersChanged();
        }
    }
}

void Bridge::tickClock()
{
    emit clockChanged();
}

void Bridge::tickMarketRefresh()
{
    if (m_marketRefreshInProgress) {
        return; // previous refresh is still in flight; skip this cycle
    }
    m_marketRefreshInProgress = true;
    market.refreshMarket();
    m_marketRefreshInProgress = false;
    emit marketChanged();
}

void Bridge::sampleIndexHistory()
{
    m_indexHistory.push_back(marketIndexValue());
    constexpr int kMaxPoints = 60;
    while (m_indexHistory.size() > kMaxPoints) {
        m_indexHistory.removeFirst();
    }
}

void Bridge::setLastError(const QString& error)
{
    m_lastError = error;
    emit lastErrorChanged();
}

// ---------------------------------------------------------------------
// Session
// ---------------------------------------------------------------------

bool Bridge::loggedIn() const { return currentUser != nullptr; }
bool Bridge::isAdmin() const { return currentUser && currentUser->isAdmin(); }
QString Bridge::username() const { return currentUser ? QString::fromStdString(currentUser->getUsername()) : QString(); }
QString Bridge::displayName() const { return currentUser ? QString::fromStdString(currentUser->getName()) : QString(); }
bool Bridge::hasUsers() { return db.hasAnyUsers(); }
QString Bridge::lastError() const { return m_lastError; }

bool Bridge::login(const QString& username, const QString& password)
{
    std::string role;
    int userId = 0;

    if (!db.verifyUser(username.toStdString(), password.toStdString(), role, userId)) {
        setLastError(QStringLiteral("Invalid username or password."));
        return false;
    }

    if (role == "ADMIN" || role == "admin") {
        currentUser = std::make_unique<Admin>(userId, "Administrator", username.toStdString(), password.toStdString());
    } else {
        currentUser = std::make_unique<Client>(userId, "Client", username.toStdString(), password.toStdString());
    }

    if (!currentUser->login(username.toStdString(), password.toStdString())) {
        currentUser.reset();
        setLastError(QStringLiteral("Login failed."));
        return false;
    }

    if (auto* client = dynamic_cast<Client*>(currentUser.get())) {
        loadClientState(client->getId(), client->getPortfolio(), client->getCollateral());
    }

    setLastError(QString());
    emit sessionChanged();
    emit portfolioChanged();
    emit ordersChanged();
    emit usersChanged();
    return true;
}

void Bridge::logout()
{
    if (currentUser) {
        currentUser->logout();
        currentUser.reset();
    }
    emit sessionChanged();
}

bool Bridge::bootstrapAdmin(const QString& name, const QString& username, const QString& password)
{
    if (name.isEmpty() || username.isEmpty() || password.isEmpty()) {
        setLastError(QStringLiteral("Name, username and password are required."));
        return false;
    }
    if (!db.addUser(name.toStdString(), username.toStdString(), password.toStdString(), "ADMIN")) {
        setLastError(QStringLiteral("Could not create admin (username may already exist)."));
        return false;
    }
    setLastError(QString());
    emit sessionChanged();
    return true;
}

// ---------------------------------------------------------------------
// Market
// ---------------------------------------------------------------------

bool Bridge::marketOpen() { return market.isMarketOpen(); }
QString Bridge::currentTime() { return QDateTime::currentDateTime().toString("hh:mm:ss AP"); }
QString Bridge::currentDate() { return QDateTime::currentDateTime().toString("ddd dd MMM yyyy"); }

QVariantList Bridge::stocks()
{
    QVariantList list;
    for (const auto& [symbol, stock] : market.getAllStocks()) {
        QVariantMap m;
        m["symbol"] = QString::fromStdString(stock.symbol.empty() ? symbol : stock.symbol);
        m["companyName"] = QString::fromStdString(stock.companyName);
        m["exchange"] = QString::fromStdString(stock.exchange);
        m["price"] = stock.close;
        m["previousClose"] = stock.previousClose;
        m["open"] = stock.openingPrice;
        m["high"] = stock.high;
        m["low"] = stock.low;
        m["percentChange"] = stock.percentChange;
        m["change"] = stock.change;
        m["volume"] = static_cast<qint64>(stock.volume);
        m["up"] = stock.percentChange >= 0.0;
        list.push_back(m);
    }
    std::sort(list.begin(), list.end(), [](const QVariant& a, const QVariant& b) {
        return a.toMap()["symbol"].toString() < b.toMap()["symbol"].toString();
    });
    return list;
}

QVariantList Bridge::topGainers()
{
    QVariantList list = stocks();
    std::sort(list.begin(), list.end(), [](const QVariant& a, const QVariant& b) {
        return a.toMap()["percentChange"].toDouble() > b.toMap()["percentChange"].toDouble();
    });
    while (list.size() > 5) list.removeLast();
    return list;
}

QVariantList Bridge::topLosers()
{
    QVariantList list = stocks();
    std::sort(list.begin(), list.end(), [](const QVariant& a, const QVariant& b) {
        return a.toMap()["percentChange"].toDouble() < b.toMap()["percentChange"].toDouble();
    });
    while (list.size() > 5) list.removeLast();
    return list;
}

void Bridge::setMarketOpen(bool open)
{
    market.setMarketStatus(open);
    emit marketChanged();
}

double Bridge::marketIndexValue()
{
    // A simple composite index (not a real NEPSE feed): starts at 1000 and
    // moves with the average percent change across all listed stocks. This
    // is deliberately synthetic since the engine only tracks individual
    // stock quotes, not a real exchange index.
    const auto all = market.getAllStocks();
    if (all.empty()) return 1000.0;
    double sum = 0.0;
    for (const auto& [symbol, stock] : all) sum += stock.percentChange;
    const double avg = sum / static_cast<double>(all.size());
    return 1000.0 * (1.0 + avg / 100.0);
}

double Bridge::marketIndexChangePercent()
{
    return marketIndexValue() - 1000.0;
}

QVariantList Bridge::marketIndexHistory()
{
    QVariantList list;
    for (int i = 0; i < m_indexHistory.size(); ++i) {
        QVariantMap p;
        p["x"] = i;
        p["y"] = m_indexHistory[i];
        list.push_back(p);
    }
    return list;
}

double Bridge::totalVolume()
{
    double total = 0.0;
    for (const auto& [symbol, stock] : market.getAllStocks()) total += stock.volume;
    return total;
}

double Bridge::totalTurnover()
{
    double total = 0.0;
    for (const auto& [symbol, stock] : market.getAllStocks()) total += stock.volume * stock.close;
    return total;
}

int Bridge::advancesCount()
{
    int count = 0;
    for (const auto& [symbol, stock] : market.getAllStocks()) if (stock.percentChange >= 0.0) ++count;
    return count;
}

int Bridge::declinesCount()
{
    int count = 0;
    for (const auto& [symbol, stock] : market.getAllStocks()) if (stock.percentChange < 0.0) ++count;
    return count;
}

int Bridge::filledTradeCount()
{
    int count = 0;
    std::vector<Order> orders;
    if (!db.loadOrders(orders)) return 0;
    for (const auto& order : orders) {
        if (order.getStatus() == OrderStatus::FILLED || order.getStatus() == OrderStatus::PARTIALLY_FILLED) ++count;
    }
    return count;
}

// ---------------------------------------------------------------------
// Portfolio / collateral
// ---------------------------------------------------------------------

double Bridge::portfolioValue()
{
    double total = 0.0;
    for (const auto& v : holdings()) {
        total += v.toMap()["marketValue"].toDouble();
    }
    return total;
}

double Bridge::portfolioPL()
{
    double total = 0.0;
    for (const auto& v : holdings()) {
        total += v.toMap()["profitLoss"].toDouble();
    }
    return total;
}

double Bridge::portfolioPLPercent()
{
    double cost = 0.0;
    for (const auto& v : holdings()) {
        const QVariantMap m = v.toMap();
        cost += m["quantity"].toInt() * m["averagePrice"].toDouble();
    }
    if (cost <= 0.0) return 0.0;
    return (portfolioPL() / cost) * 100.0;
}

double Bridge::collateralAvailable()
{
    if (!currentUser) return 0.0;
    double available = 0.0, blocked = 0.0, total = 0.0;
    db.loadCollateral(currentUser->getId(), available, blocked, total);
    return available;
}

double Bridge::collateralBlocked()
{
    if (!currentUser) return 0.0;
    double available = 0.0, blocked = 0.0, total = 0.0;
    db.loadCollateral(currentUser->getId(), available, blocked, total);
    return blocked;
}

double Bridge::collateralTotal()
{
    if (!currentUser) return 0.0;
    double available = 0.0, blocked = 0.0, total = 0.0;
    db.loadCollateral(currentUser->getId(), available, blocked, total);
    return total;
}

QVariantList Bridge::holdings()
{
    QVariantList list;
    if (!currentUser) return list;

    std::vector<std::tuple<std::string, int, double>> rows;
    if (!db.loadPortfolioDetailed(currentUser->getId(), rows)) return list;

    for (const auto& [symbol, quantity, averagePrice] : rows) {
        const double lastPrice = market.getCurrentPrice(symbol);
        const double marketValue = lastPrice * quantity;
        const double pl = (lastPrice - averagePrice) * quantity;

        QVariantMap m;
        m["symbol"] = QString::fromStdString(symbol);
        m["quantity"] = quantity;
        m["averagePrice"] = averagePrice;
        m["lastPrice"] = lastPrice;
        m["marketValue"] = marketValue;
        m["profitLoss"] = pl;
        m["up"] = pl >= 0.0;
        const double changePercent = averagePrice > 0.0 ? ((lastPrice - averagePrice) / averagePrice) * 100.0 : 0.0;
        m["changePercent"] = changePercent;
        list.push_back(m);
    }
    return list;
}

QVariantList Bridge::myOrders()
{
    QVariantList list;
    if (!currentUser) return list;
    std::vector<Order> orders;
    if (!db.loadOrders(orders)) return list;
    for (const auto& order : orders) {
        if (order.getUserId() == currentUser->getId()) {
            list.push_back(orderToMap(order));
        }
    }
    return list;
}

int Bridge::openOrderCount()
{
    int count = 0;
    for (const auto& v : myOrders()) {
        if (v.toMap()["isOpen"].toBool()) ++count;
    }
    return count;
}

bool Bridge::placeBuyOrder(const QString& symbol, int quantity, double price)
{
    auto* client = dynamic_cast<Client*>(currentUser.get());
    if (client == nullptr) { setLastError(QStringLiteral("Only a client can place orders.")); return false; }
    loadClientState(client->getId(), client->getPortfolio(), client->getCollateral());
    if (!client->placeBuyOrder(symbol.toStdString(), quantity, price)) {
        setLastError(QStringLiteral("Could not place buy order — check quantity, price and available collateral."));
        return false;
    }
    setLastError(QString());
    runMatchingEngine();
    emit portfolioChanged();
    emit ordersChanged();
    return true;
}

bool Bridge::placeSellOrder(const QString& symbol, int quantity, double price)
{
    auto* client = dynamic_cast<Client*>(currentUser.get());
    if (client == nullptr) { setLastError(QStringLiteral("Only a client can place orders.")); return false; }
    loadClientState(client->getId(), client->getPortfolio(), client->getCollateral());
    if (!client->placeSellOrder(symbol.toStdString(), quantity, price)) {
        setLastError(QStringLiteral("Could not place sell order — check that you own enough shares."));
        return false;
    }
    setLastError(QString());
    runMatchingEngine();
    emit portfolioChanged();
    emit ordersChanged();
    return true;
}

bool Bridge::cancelOrder(int orderId)
{
    auto* client = dynamic_cast<Client*>(currentUser.get());
    if (client == nullptr) { setLastError(QStringLiteral("Only a client can cancel orders.")); return false; }
    loadClientState(client->getId(), client->getPortfolio(), client->getCollateral());
    if (!client->cancelOrder(orderId)) {
        setLastError(QStringLiteral("Could not cancel order."));
        return false;
    }
    emit ordersChanged();
    return true;
}

bool Bridge::depositCollateral(double amount)
{
    auto* client = dynamic_cast<Client*>(currentUser.get());
    if (client == nullptr) { setLastError(QStringLiteral("Only a client can deposit collateral.")); return false; }
    loadClientState(client->getId(), client->getPortfolio(), client->getCollateral());
    if (!client->depositCollateral(amount)) {
        setLastError(QStringLiteral("Deposit failed — amount must be positive."));
        return false;
    }
    emit portfolioChanged();
    return true;
}

bool Bridge::withdrawCollateral(double amount)
{
    auto* client = dynamic_cast<Client*>(currentUser.get());
    if (client == nullptr) { setLastError(QStringLiteral("Only a client can withdraw collateral.")); return false; }
    loadClientState(client->getId(), client->getPortfolio(), client->getCollateral());
    if (!client->withdrawCollateral(amount)) {
        setLastError(QStringLiteral("Withdraw failed — insufficient available balance."));
        return false;
    }
    emit portfolioChanged();
    return true;
}

// ---------------------------------------------------------------------
// Admin
// ---------------------------------------------------------------------

QVariantList Bridge::allUsers()
{
    QVariantList list;
    std::vector<std::tuple<int, std::string, std::string, std::string>> users;
    if (!db.loadUsers(users)) return list;
    for (const auto& u : users) {
        QVariantMap m;
        m["id"] = std::get<0>(u);
        m["name"] = QString::fromStdString(std::get<1>(u));
        m["username"] = QString::fromStdString(std::get<2>(u));
        m["role"] = QString::fromStdString(std::get<3>(u));
        list.push_back(m);
    }
    return list;
}

QVariantList Bridge::allOrders()
{
    QVariantList list;
    std::vector<Order> orders;
    if (!db.loadOrders(orders)) return list;
    for (const auto& order : orders) {
        list.push_back(orderToMap(order));
    }
    return list;
}

int Bridge::userCount() { return allUsers().size(); }
int Bridge::orderCount() { return allOrders().size(); }

bool Bridge::addUser(const QString& name, const QString& username, const QString& password, const QString& role)
{
    auto* admin = dynamic_cast<Admin*>(currentUser.get());
    if (admin == nullptr) { setLastError(QStringLiteral("Only an administrator can add users.")); return false; }
    QString normalizedRole = role.toUpper();
    if (normalizedRole != "ADMIN" && normalizedRole != "CLIENT") {
        setLastError(QStringLiteral("Role must be ADMIN or CLIENT."));
        return false;
    }
    if (!admin->addUser(db, name.toStdString(), username.toStdString(), password.toStdString(), normalizedRole.toStdString())) {
        setLastError(QStringLiteral("Could not add user (username may already exist)."));
        return false;
    }
    emit usersChanged();
    return true;
}

bool Bridge::removeUser(int userId)
{
    auto* admin = dynamic_cast<Admin*>(currentUser.get());
    if (admin == nullptr) { setLastError(QStringLiteral("Only an administrator can remove users.")); return false; }
    if (!admin->removeUser(db, userId)) {
        setLastError(QStringLiteral("Could not remove user."));
        return false;
    }
    emit usersChanged();
    return true;
}

bool Bridge::resetPassword(int userId, const QString& newPassword)
{
    auto* admin = dynamic_cast<Admin*>(currentUser.get());
    if (admin == nullptr) { setLastError(QStringLiteral("Only an administrator can reset passwords.")); return false; }
    if (newPassword.isEmpty()) { setLastError(QStringLiteral("Password cannot be empty.")); return false; }
    if (!admin->resetPassword(db, userId, newPassword.toStdString())) {
        setLastError(QStringLiteral("Could not reset password."));
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------
// Internal
// ---------------------------------------------------------------------

bool Bridge::loadClientState(int clientId, Portfolio& portfolio, Collateral& collateral)
{
    std::vector<std::tuple<std::string, int, double>> rows;
    if (!db.loadPortfolioDetailed(clientId, rows)) return false;

    portfolio.clear();
    for (const auto& [symbol, quantity, averagePrice] : rows) {
        portfolio.setHolding(symbol, quantity, averagePrice);
    }

    double available = 0.0, blocked = 0.0, total = 0.0;
    if (!db.loadCollateral(clientId, available, blocked, total)) return true;

    while (collateral.getTotalCollateral() > 0.0) {
        if (collateral.getBlockedCollateral() > 0.0) collateral.release(collateral.getBlockedCollateral());
        collateral.withdraw(collateral.getAvailableCollateral());
    }
    if (available > 0.0) collateral.deposit(available);
    if (blocked > 0.0) { collateral.deposit(blocked); collateral.reserve(blocked); }

    return true;
}

void Bridge::runMatchingEngine()
{
    std::vector<Order> databaseOrders;
    if (!db.loadOrders(databaseOrders)) return;

    orderBook.clear();
    for (const auto& order : databaseOrders) {
        if (order.getStatus() == OrderStatus::PENDING || order.getStatus() == OrderStatus::PARTIALLY_FILLED) {
            orderBook.addOrder(order);
        }
    }

    std::vector<Order> buyOrders = orderBook.getBuyOrders();
    std::vector<Order> sellOrders = orderBook.getSellOrders();

    bool executedAny = false;
    while (true) {
        MatchResult match = matchmaking.matchOrders(buyOrders, sellOrders);
        if (!match.matched || match.buyOrder == nullptr || match.sellOrder == nullptr) break;

        Portfolio buyerPortfolio(match.buyOrder->getUserId());
        Portfolio sellerPortfolio(match.sellOrder->getUserId());
        Collateral buyerCollateral(match.buyOrder->getUserId());
        Collateral sellerCollateral(match.sellOrder->getUserId());

        if (!loadClientState(match.buyOrder->getUserId(), buyerPortfolio, buyerCollateral)
            || !loadClientState(match.sellOrder->getUserId(), sellerPortfolio, sellerCollateral)) {
            return;
        }

        const bool executed = tradeExecution.executeTrade(
            *match.buyOrder, *match.sellOrder,
            buyerPortfolio, sellerPortfolio,
            buyerCollateral, sellerCollateral);

        if (!executed) {
            match.buyOrder->setStatus(OrderStatus::FAILED);
            match.sellOrder->setStatus(OrderStatus::FAILED);
            db.updateOrderStatus(match.buyOrder->getOrderId(), OrderStatus::FAILED, match.buyOrder->getRemainingQuantity());
            db.updateOrderStatus(match.sellOrder->getOrderId(), OrderStatus::FAILED, match.sellOrder->getRemainingQuantity());
            continue;
        }
        executedAny = true;
    }

    if (executedAny) {
        emit portfolioChanged();
        emit ordersChanged();
    }
}
