#ifndef TRADING_SYSTEM_H
#define TRADING_SYSTEM_H

#include <memory>

#include "../market/market.h"
#include "../execution/tradeExecution.h"    
#include "../Matchmaking/matchmaking.h"
#include "../orderbook/orderBook.hpp"
#include "../users/user.h"
#include "../database/db.hpp"

class TradingSystem {
    private:
        Database db;
        Market market;
        TradeExecution tradeExecution;
        MatchMaking matchmaking;
        OrderBook orderBook;

        std::unique_ptr<User> currentUser;

        bool loadClientState(int clientId, Portfolio& portfolio, Collateral& collateral);
        bool runMatchingEngine();
        void runAdminSession();
        void runClientSession();

    public:
        TradingSystem();
        

        bool initializeSystem();
        void login();
        void run();
        void shutdown();

};


#endif