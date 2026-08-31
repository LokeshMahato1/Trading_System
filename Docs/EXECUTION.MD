#include "tradeExecution.h"

#include "../database/db.hpp"

#include <algorithm>

namespace {

bool syncPortfolioToDatabase(Database& db, const Portfolio& portfolio, const std::string& symbol)
{
    return db.updatePortfolio(
        portfolio.getClientId(),
        symbol,
        portfolio.getQuantity(symbol),
        portfolio.getAveragePrice(symbol)
    );
}

bool syncCollateralToDatabase(Database& db, const Collateral& collateral)
{
    return db.updateCollateral(
        collateral.getClientId(),
        collateral.getAvailableCollateral(),
        collateral.getBlockedCollateral(),
        collateral.getTotalCollateral()
    );
}

} // namespace

TradeExecution::TradeExecution(Database& db)
    : database(db)
{}

bool TradeExecution::executeTrade(
    Order& buyOrder,
    Order& sellOrder,
    Portfolio& buyerPortfolio,
    Portfolio& sellerPortfolio,
    Collateral& buyerCollateral,
    Collateral& sellerCollateral
)
{
    if (!buyOrder.isBuyOrder() || !sellOrder.isSellOrder()) {
        return false;
    }

    if (buyOrder.getStockSymbol() != sellOrder.getStockSymbol()) {
        return false;
    }

    if (buyOrder.getPrice() < sellOrder.getPrice()) {
        return false;
    }

    const int matchedQuantity = std::min(
        buyOrder.getRemainingQuantity(),
        sellOrder.getRemainingQuantity()
    );

    if (matchedQuantity <= 0) {
        return false;
    }

    const std::string symbol = buyOrder.getStockSymbol();
    const double executionPrice = sellOrder.getPrice();
    const double tradeValue = calculateTradeValue(matchedQuantity, executionPrice);

    if (!updateBuyerPortfolio(buyerPortfolio, symbol, matchedQuantity, executionPrice)) {
        return false;
    }

    if (!updateSellerPortfolio(sellerPortfolio, symbol, matchedQuantity)) {
        buyerPortfolio.sellStock(symbol, matchedQuantity);
        return false;
    }

    if (!updateBuyerCollateral(buyerCollateral, buyOrder, matchedQuantity, tradeValue)) {
        buyerPortfolio.sellStock(symbol, matchedQuantity);
        sellerPortfolio.buyStock(symbol, matchedQuantity, executionPrice);
        return false;
    }

    updateSellerCollateral(sellerCollateral, tradeValue);

    updateOrder(buyOrder, matchedQuantity);
    updateOrder(sellOrder, matchedQuantity);

    if (!database.open()) {
        return false;
    }

    bool success = true;

    success = success && database.updateOrderStatus(
        buyOrder.getOrderId(),
        buyOrder.getStatus(),
        buyOrder.getRemainingQuantity()
    );

    success = success && database.updateOrderStatus(
        sellOrder.getOrderId(),
        sellOrder.getStatus(),
        sellOrder.getRemainingQuantity()
    );

    success = success && syncPortfolioToDatabase(database, buyerPortfolio, symbol);
    success = success && syncPortfolioToDatabase(database, sellerPortfolio, symbol);

    success = success && syncCollateralToDatabase(database, buyerCollateral);
    success = success && syncCollateralToDatabase(database, sellerCollateral);

    success = success && database.saveTransaction(
        buyOrder.getUserId(),
        sellOrder.getUserId(),
        buyOrder,
        matchedQuantity,
        executionPrice
    );

    return success;
}

double TradeExecution::calculateTradeValue(
    int quantity,
    double executionPrice
) const
{
    return quantity * executionPrice;
}

bool TradeExecution::updateBuyerPortfolio(
    Portfolio& buyerPortfolio,
    const std::string& symbol,
    int quantity,
    double executionPrice
)
{
    return buyerPortfolio.buyStock(symbol, quantity, executionPrice);
}

bool TradeExecution::updateSellerPortfolio(
    Portfolio& sellerPortfolio,
    const std::string& symbol,
    int quantity
)
{
    return sellerPortfolio.sellStock(symbol, quantity);
}

bool TradeExecution::updateBuyerCollateral(
    Collateral& buyerCollateral,
    const Order& buyOrder,
    int matchedQuantity,
    double tradeValue
)
{
    if (tradeValue <= 0.0) {
        return false;
    }

    constexpr double EPSILON = 1e-6;
    if (buyerCollateral.getBlockedCollateral() + EPSILON < tradeValue) {
        return false;
    }
    
    const double deductedAmount = std::min( tradeValue, buyerCollateral.getBlockedCollateral());
    
    if (!buyerCollateral.deduct(deductedAmount)) {
        return false;
    }

    const double priceDifferencePerUnit = buyOrder.getPrice() - (tradeValue / matchedQuantity);
    const double rebate=priceDifferencePerUnit>0.0 ? priceDifferencePerUnit * matchedQuantity : 0.0;

    if(rebate > 0.0) {
        buyerCollateral.release(rebate);
    }

    return true;
}

void TradeExecution::updateSellerCollateral(Collateral& sellerCollateral, double tradeValue)
{
    sellerCollateral.deposit(tradeValue);
}

void TradeExecution::updateOrder(Order& order, int executedQuantity)
{
    const int remainingQuantity = order.getRemainingQuantity() - executedQuantity;
    order.setRemainingQuantity(remainingQuantity);

    if (remainingQuantity == 0)
    {
        order.setStatus(OrderStatus::FILLED);
    }
    else
    {
        order.setStatus(OrderStatus::PARTIALLY_FILLED);
    }
}
