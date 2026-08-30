#include "client.h"

#include "../database/db.hpp"
#include "../order/order.h"

#include <atomic>
#include <chrono>
#include <iostream>

namespace {

int generateOrderId()
{
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const int seed = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now).count());
    static std::atomic<int> nextOrderId(seed);
    return nextOrderId.fetch_add(1);
}

bool loadUserOrder(Database& db, int userId, int orderId, Order& outOrder)
{
    std::vector<Order> orders;
    if (!db.loadOrders(orders)) {
        return false;
    }

    for (const auto& order : orders) {
        if (order.getOrderId() == orderId && order.getUserId() == userId) {
            outOrder = order;
            return true;
        }
    }

    return false;
}

bool syncCollateralToDb(Database& db, int clientId, const Collateral& collateral)
{
    return db.updateCollateral(
        clientId,
        collateral.getAvailableCollateral(),
        collateral.getBlockedCollateral(),
        collateral.getTotalCollateral()
    );
}

}

Client::Client(
    int id,
    const std::string& name,
    const std::string& username,
    const std::string& password
) : User(id, name, username, password, UserRole::CLIENT),
    portfolio(id),
    collateral(id) {}

//portffolio

Portfolio& Client::getPortfolio()
{
    return portfolio;
}

const Portfolio& Client::getPortfolio() const
{
    return portfolio;
}

//collateral

Collateral& Client::getCollateral()
{
    return collateral;
}

const Collateral& Client::getCollateral() const
{
    return collateral;
}

//trading operations

bool Client::placeBuyOrder(
    const std::string& stockSymbol,
    int quantity,
    double price
)
{
    if (stockSymbol.empty() || quantity <= 0 || price <= 0.0) {
        return false;
    }

    Database db;
    if (!db.open()) {
        return false;
    }

    const double requiredAmount = price * quantity;
    if (!collateral.reserve(requiredAmount)) {
        return false;
    }

    if (!syncCollateralToDb(db, getId(), collateral)) {
        collateral.release(requiredAmount);
        return false;
    }

    const Order order(
        generateOrderId(),
        getId(),
        stockSymbol,
        quantity,
        price,
        OrderType::BUY
    );

    if (!db.saveOrder(order)) {
        collateral.release(requiredAmount);
        syncCollateralToDb(db, getId(), collateral);
        return false;
    }

    return true;
}

bool Client::placeSellOrder(
    const std::string& stockSymbol,
    int quantity,
    double price
)
{
    if (stockSymbol.empty() || quantity <= 0 || price <= 0.0) {
        return false;
    }

    if (portfolio.getQuantity(stockSymbol) < quantity) {
        return false;
    }

    Database db;
    if (!db.open()) {
        return false;
    }

    const Order order(
        generateOrderId(),
        getId(),
        stockSymbol,
        quantity,
        price,
        OrderType::SELL
    );

    return db.saveOrder(order);
}

bool Client::cancelOrder(int orderId)
{
    if (orderId <= 0) {
        return false;
    }

    Database db;
    if (!db.open()) {
        return false;
    }

    Order existingOrder;
    if (!loadUserOrder(db, getId(), orderId, existingOrder)) {
        return false;
    }

    if(!db.cancelOrder(orderId)) {
        return false;
    }

    if(existingOrder.getType() == OrderType::BUY) {
        const double outstandingAmount = existingOrder.getPrice() * existingOrder.getRemainingQuantity();

        if(outstandingAmount > 0.0) {
            collateral.release(outstandingAmount);
            syncCollateralToDb(db, getId(), collateral);
        }
    }
    return true;
}

bool Client::modifyOrder(int orderId, int newQuantity, double newPrice)
{
    if (orderId <= 0 || newQuantity <= 0 || newPrice <= 0.0) {
        return false;
    }

    Database db;
    if (!db.open()) {
        return false;
    }

    Order existingOrder;
    if (!loadUserOrder(db, getId(), orderId, existingOrder)) {
        return false;
    }

    const bool isBuy=existingOrder.getType() == OrderType::BUY;

    if(isBuy) {
        const double oldAmount = existingOrder.getPrice() * existingOrder.getRemainingQuantity();
        const double newAmount = newPrice * newQuantity;

        collateral.release(oldAmount);

        if(!collateral.reserve(newAmount)) {
            collateral.reserve(oldAmount);
            return false;
        }

        if (!syncCollateralToDb(db, getId(), collateral)) {
            collateral.release(newAmount);
            collateral.reserve(oldAmount);
            return false;
        }

    }

    else{
        if(portfolio.getQuantity(existingOrder.getStockSymbol()) < newQuantity) {
            return false;
        }
    }

    if(!cancelOrder(orderId)) {
        if(isBuy) {
            const double oldAmount = existingOrder.getPrice() * existingOrder.getRemainingQuantity();
            const double newAmount = newPrice * newQuantity;
            collateral.release(newAmount);
            collateral.reserve(oldAmount);
            syncCollateralToDb(db, getId(), collateral);
        }
        return false;
    }

    const Order replacementOrder(
        generateOrderId(),
        getId(),
        existingOrder.getStockSymbol(),
        newQuantity,
        newPrice,
        existingOrder.getType()
    );

    if(!db.saveOrder(replacementOrder)) {
        if(isBuy) {
            const double newAmount = newPrice * newQuantity;
            collateral.release(newAmount);
            syncCollateralToDb(db, getId(), collateral);
        }
        return false;
    }
    return true;
}


bool Client::depositCollateral(double amount)
{
    if (amount <= 0.0) {
        return false;
    }

    if (!collateral.deposit(amount)) {
        return false;
    }

    Database db;
    if (!db.open()) {
        collateral.withdraw(amount);
        return false;
    }

    if (!syncCollateralToDb(db, getId(), collateral)) {
        collateral.withdraw(amount);
        return false;
    }

    return true;
}

bool Client::withdrawCollateral(double amount)
{
    if (amount <= 0.0) {
        return false;
    }

    if (!collateral.withdraw(amount)) {
        return false;
    }

    Database db;
    if (!db.open()) {
        collateral.deposit(amount);
        return false;
    }

    if (!syncCollateralToDb(db, getId(), collateral)) {
        collateral.deposit(amount);
        return false;
    }

    return true;
}

void Client::viewPortfolio() const
{
    portfolio.displayPortfolio();
}

void Client::viewCollateral() const
{
    std::cout << "Total: "
              << collateral.getTotalCollateral()
              << std::endl;
    std::cout << "Available: "
              << collateral.getAvailableCollateral()
              << std::endl;
    std::cout << "Blocked: "
              << collateral.getBlockedCollateral()
              << std::endl;
}

//client menu display function
void Client::displayMenu()
{
    std::cout << "========== Client Menu ==========" << std::endl;
    std::cout << "1. View Portfolio" << std::endl;
    std::cout << "2. Place Buy Order" << std::endl;
    std::cout << "3. Place Sell Order" << std::endl;
    std::cout << "4. View Collateral" << std::endl;
    std::cout << "5. Deposit Collateral" << std::endl;
    std::cout << "6. Withdraw Collateral" << std::endl;
    std::cout << "7. Modify Order" << std::endl;
    std::cout << "8. Cancel Order" << std::endl;
    std::cout << "9. Logout" << std::endl;
}