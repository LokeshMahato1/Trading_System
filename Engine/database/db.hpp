// import the model of the stock data
#ifndef DB_HPP
#define DB_HPP

#include <string>
#include <tuple>
#include <vector>

#include <sqlite3.h>

#include "../model/models.hpp"
#include "../order/order.h"
class Database{

    private:
        sqlite3* db;
        const std::string dbName;
        bool executeSQL(const std::string& sql);

        bool userExists(int userId);
        bool stockExists(const std::string& symbol);
        bool orderExists(int orderId);

    public:
         // create the instance for the name of the name of the database.......Construction of the database
        Database();
        // destroy the instance of the database
        ~Database();

        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;

        //connection
        bool open();
        bool close();


    // add the data of the certain struct to add in the table
        bool createTables();


    // add the data of the stocks to the table of stocks
        bool addStocks(const std::vector<StockData>& stocks);
        bool loadStocks(std::vector<StockData>& stocks);

        //user operations

        bool verifyUser(const std::string& username,
    
             const std::string& password,
             std::string& role,
             int& userId
            );

        bool addUser(const std::string& name,
            const std::string& username,
             const std::string& password,
             const std::string& role
            );

        bool removeUser(int userId);
        bool loadUsers(std::vector<std::tuple<int, std::string, std::string, std::string>>& users);
        bool hasAnyUsers();

        //order operations

        bool saveOrder(const Order& order);
        bool loadOrders(std::vector<Order>& orders);

        //portfolio operations

        bool loadPortfolio(int clientId, std::vector<std::pair<std::string,int>>& holdings);
        bool loadPortfolioDetailed(int clientId, std::vector<std::tuple<std::string, int, double>>& holdings);
        bool updatePortfolio(int clientId, const std::string& symbol, int quantity, double averagePrice);

        //collateral operations

        bool loadCollateral(int clientId, double& available, double& blocked, double& total);
        bool updateCollateral(int clientId, double available, double blocked, double total);

        //transaction operations

        bool saveTransaction(
            int buyerId,
            int sellerId,
            const Order& order,
            int quantity,
            double executionPrice
        );

        bool changePassword(int userId, const std::string& newPassword);

        bool updateOrderStatus(int orderId, OrderStatus status, int remainingQuantity);

        bool cancelOrder(int orderId);


};
#endif
