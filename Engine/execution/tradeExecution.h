#ifndef TRADE_EXECUTION_H
#define TRADE_EXECUTION_H


#include "../order/order.h"
#include "../portfolio/portfolio.h"
#include "../collateral/collateral.h"

class Database;

class TradeExecution {
public:
    explicit TradeExecution(Database& db);

    bool executeTrade(
        Order& buyOrder,
        Order& sellOrder,
        Portfolio& buyerPortfolio,
        Portfolio& sellerPortfolio,
        Collateral& buyerCollateral,
        Collateral& sellerCollateral
    );

private:
    Database& database;

        //calculate total trade value

        double calculateTradeValue(
            int quantity,
            double executionPrice
        ) const;

        //update portfolio

        bool updateBuyerPortfolio(
            Portfolio& buyerPortfolio,
            const std::string& symbol,
            int quantity,
            double executionPrice
        );

        bool updateSellerPortfolio(
            Portfolio& sellerPortfolio,
            const std::string& symbol,
            int quantity
        );

        //update collateral

        bool updateBuyerCollateral(
            Collateral& buyerCollateral,
            const Order& buyOrder,
            int matchedQuantity,
            double tradeValue
        );

        void updateSellerCollateral(Collateral& sellerCollateral, double tradeValue);

        //update trade information

        void updateOrder(Order& order, int executedQuantity);


};

#endif

