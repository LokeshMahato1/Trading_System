#include "db.hpp"

#include "../util/passwordHash.hpp"
#include <iostream>

//database constructor and destructor
Database::Database()
    : db(nullptr),
    dbName("Trading_Management_System.db")
{
}

Database::~Database()
{
    close();
}
//opening the database connection
bool Database::open()
{
    if(db!=nullptr){
        return true; // Database is already open
    }

    int rc=sqlite3_open(dbName.c_str(), &db);

    if(rc!=SQLITE_OK){
        std::cerr<<"Failed to open database: "
        <<sqlite3_errmsg(db)
        <<std::endl;
        
        sqlite3_close(db);
        db=nullptr;
        return false;
    }

    char* errMsg=nullptr;
    if(sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsg)!=SQLITE_OK){
        std::cerr<<"Failed to enable foreign key support: "
        <<errMsg
        <<std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        db=nullptr;
        return false;
    }

    std::cout<<"Database Connected Successfully."<<std::endl;
    return true;
}


//closing the database connection

bool Database::close()
{
    if(db==nullptr){
        return true; // Database is already closed
    }

    sqlite3_close(db);
    db=nullptr;

    return true;
}
//executing the sql command

bool Database::executeSQL(const std::string& sql)
{
    char* errMsg=nullptr;
    int rc=sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);

    if(rc!=SQLITE_OK){
        std::cerr<<"SQLite Error: "
        <<errMsg
        <<std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool Database::userExists(int userId)
{
    const char* sql =
        "SELECT 1 FROM users WHERE userId = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_int(stmt, 1, userId);

    bool exists = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        exists = true;
    }

    sqlite3_finalize(stmt);

    return exists;
}

bool Database::stockExists(const std::string& symbol)
{
    const char* sql =
        "SELECT 1 FROM stocks WHERE symbol = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_text(
        stmt,
        1,
        symbol.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    bool exists = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        exists = true;
    }

    sqlite3_finalize(stmt);

    return exists;
}

bool Database::orderExists(int orderId)
{
    const char* sql =
        "SELECT 1 FROM orders WHERE orderId = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_int(stmt, 1, orderId);

    bool exists = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        exists = true;
    }

    sqlite3_finalize(stmt);

    return exists;
}

//creating the tables in the database
bool Database::createTables()
{

    if(!open()){
        return false;
    }
    const char* sql=
        R"(

CREATE TABLE IF NOT EXISTS users(

userId INTEGER PRIMARY KEY AUTOINCREMENT,

name TEXT NOT NULL,

username TEXT UNIQUE NOT NULL,

password TEXT NOT NULL,

role TEXT NOT NULL

);

CREATE TABLE IF NOT EXISTS stocks(

symbol TEXT PRIMARY KEY,

companyName TEXT,

exchangeName TEXT,

currency TEXT,

price REAL,

open REAL,

high REAL,

low REAL,

previousClose REAL,

change REAL,

percentChange REAL,

volume INTEGER,

marketOpen INTEGER,

date TEXT

);

CREATE TABLE IF NOT EXISTS orders(

orderId INTEGER PRIMARY KEY,

userId INTEGER NOT NULL,

symbol TEXT NOT NULL,

quantity INTEGER NOT NULL,

remainingQuantity INTEGER NOT NULL,

price REAL NOT NULL,

type INTEGER NOT NULL,

status INTEGER NOT NULL,

createdAt DATETIME DEFAULT CURRENT_TIMESTAMP,

FOREIGN KEY(userId) REFERENCES users(userId),

FOREIGN KEY(symbol) REFERENCES stocks(symbol)

);

CREATE TABLE IF NOT EXISTS portfolio(

clientId INTEGER,

symbol TEXT,

quantity INTEGER,

averagePrice REAL,

PRIMARY KEY(clientId,symbol),

FOREIGN KEY(clientId) REFERENCES users(userId),

FOREIGN KEY(symbol) REFERENCES stocks(symbol)

);

CREATE TABLE IF NOT EXISTS collateral(

clientId INTEGER PRIMARY KEY,

availableBalance REAL,

blockedBalance REAL,

totalBalance REAL,

FOREIGN KEY(clientId) REFERENCES users(userId)

);

CREATE TABLE IF NOT EXISTS transactions(

transactionId INTEGER PRIMARY KEY AUTOINCREMENT,

buyerId INTEGER,

sellerId INTEGER,

symbol TEXT,

quantity INTEGER,

price REAL,

tradeTime DATETIME DEFAULT CURRENT_TIMESTAMP,

FOREIGN KEY(buyerId) REFERENCES users(userId),

FOREIGN KEY(sellerId) REFERENCES users(userId),

FOREIGN KEY(symbol) REFERENCES stocks(symbol)

);

)";
   return executeSQL(sql);
}

//adding the stocks to the database

bool Database::addStocks(const std::vector<StockData>& stocks)
{
    if(!open()){
        return false;
    }

    const char* sql=
        "INSERT OR REPLACE INTO STOCKS("
        "symbol, companyName, exchangeName, currency, price, open, high, low,"
        "previousClose, change, percentChange, volume, marketOpen, date)"
        "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)";

    sqlite3_stmt* stmt=nullptr;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK){
        std::cerr<<"Failed to prepare AddStock statement: "
        <<std::endl;
        return false;
    }

    for(const auto& stock : stocks)
    {
        sqlite3_bind_text(stmt, 1, stock.symbol.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, stock.companyName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, stock.exchange.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, stock.currency.c_str(), -1, SQLITE_TRANSIENT);


        sqlite3_bind_double(stmt, 5, stock.close);
        sqlite3_bind_double(stmt, 6, stock.openingPrice);
        sqlite3_bind_double(stmt, 7, stock.high);
        sqlite3_bind_double(stmt, 8, stock.low);

        sqlite3_bind_double(stmt, 9, stock.previousClose);
        sqlite3_bind_double(stmt, 10, stock.change);
        sqlite3_bind_double(stmt, 11, stock.percentChange);


        sqlite3_bind_int(stmt, 12, stock.volume);
        sqlite3_bind_int(stmt, 13, stock.isMarketOpen);
        sqlite3_bind_text(stmt, 14, stock.date.c_str(), -1, SQLITE_TRANSIENT);


        if(sqlite3_step(stmt)!=SQLITE_DONE){
            std::cerr<<"Error inserting stock"
            <<stock.symbol
            <<std::endl;
        }

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }
    sqlite3_finalize(stmt);
    return true;
}

//loading the stocks from the database
bool Database::loadStocks(std::vector<StockData>& stocks)
{
    if(!open()){
        return false;
    }

    stocks.clear();

    const char* sql="SELECT * FROM STOCKS";

    sqlite3_stmt* stmt=nullptr;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK){
        std::cerr<<"Failed to prepare LoadStocks statement: "
        <<std::endl;
        return false;
    }

    while (sqlite3_step(stmt)==SQLITE_ROW)
    {
        StockData stock;

        stock.symbol=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        stock.companyName=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        stock.exchange=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        stock.currency=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        stock.close=sqlite3_column_double(stmt, 4);
        stock.openingPrice=sqlite3_column_double(stmt, 5);
        stock.high=sqlite3_column_double(stmt, 6);
        stock.low=sqlite3_column_double(stmt, 7);

        stock.previousClose=sqlite3_column_double(stmt, 8);
        stock.change=sqlite3_column_double(stmt, 9);
        stock.percentChange=sqlite3_column_double(stmt, 10);

        stock.volume=sqlite3_column_int(stmt, 11);
        stock.isMarketOpen=static_cast<bool>(sqlite3_column_int(stmt, 12));
        stock.date=std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13)));

        stocks.push_back(stock);
    }
    sqlite3_finalize(stmt);
    return true;
}

//verifying the user credentials
bool Database::verifyUser(
    const std::string& username,
    const std::string& password,
    std::string& role,
    int& userId
    )
{
    if(!open()){
        return false;
    }

    const char* sql="SELECT userId, role, password FROM users WHERE username=? ";

    sqlite3_stmt* stmt=nullptr;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK){
        std::cerr<<"Failed to prepare login statement: "
        <<std::endl;
        return false;
    }


    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    bool success=false;

    if(sqlite3_step(stmt)==SQLITE_ROW)
    {
        const int candidateUserId=sqlite3_column_int(stmt, 0);
        const std::string candidateRole=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const std::string storedPasswordHash=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        if(passwordHash::verifyPassword(password, storedPasswordHash))
        {
            userId=candidateUserId;
            role=candidateRole;
            success=true;
        }
    }

    sqlite3_finalize(stmt);
    return success;
}


//adding the user to the database
bool Database::addUser(
    const std::string& name,
    const std::string& username,
    const std::string& password,
    const std::string& role
    )
{
    if(!open()){
        return false;
    }

    const char* sql="INSERT INTO users(name, username, password, role) VALUES(?,?,?,?)";

    sqlite3_stmt* stmt=nullptr;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK){
        std::cerr<<"Failed to prepare addUser statement: "
        <<std::endl;
        return false;
    }
    const std::string hashedPassword=passwordHash::hashPassword(password);

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, role.c_str(), -1, SQLITE_TRANSIENT);

    bool success=true;

    if(sqlite3_step(stmt)!=SQLITE_DONE){
        std::cerr<<"User already exists: "
        <<std::endl;
        success=false;
    }

    sqlite3_finalize(stmt);
    return success;
}

//removing the user from the database
bool Database::removeUser(int userId)
{
    if (!open())
        return false;

    const char* sql =
        "DELETE FROM users WHERE userId = ?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, userId);

    bool success =
        sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);

    return success;
}

bool Database::loadUsers(std::vector<std::tuple<int, std::string, std::string, std::string>>& users)
{
    if (!open())
        return false;

    users.clear();

    const char* sql = "SELECT userId, name, username, role FROM users ORDER BY userId";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const int userId = sqlite3_column_int(stmt, 0);
        const char* nameText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* usernameText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        const char* roleText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        users.emplace_back(
            userId,
            nameText ? nameText : "",
            usernameText ? usernameText : "",
            roleText ? roleText : ""
        );
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::hasAnyUsers()
{
    if (!open())
        return false;

    const char* sql = "SELECT EXISTS(SELECT 1 FROM users LIMIT 1);";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    bool anyUsers = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        anyUsers = sqlite3_column_int(stmt, 0) != 0;
    }

    sqlite3_finalize(stmt);
    return anyUsers;
}



//changing the password of the user
bool Database::changePassword(
    int userId,
    const std::string& newPassword)
{
    if (!open())
        return false;

    const char* sql =
        "UPDATE users "
        "SET password=? "
        "WHERE userId=?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    const std::string passwordToStore = passwordHash::hashPassword(newPassword);

    sqlite3_bind_text(stmt, 1, passwordToStore.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, userId);

    bool success =
        sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);

    return success;
}

//updating the order status in the database
bool Database::saveOrder(const Order& order)
{
    if (!userExists(order.getUserId()))
    {
        return false;
    }

    if (!stockExists(order.getStockSymbol()))
    {
        return false;
    }

    const char* sql =
        "INSERT INTO orders "
        "(orderId, userId, symbol, quantity, remainingQuantity, price, type, status) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: "
                  << sqlite3_errmsg(db)
                  << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, order.getOrderId());
    sqlite3_bind_int(stmt, 2, order.getUserId());
    sqlite3_bind_text(stmt, 3, order.getStockSymbol().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, order.getQuantity());
    sqlite3_bind_int(stmt, 5, order.getRemainingQuantity());
    sqlite3_bind_double(stmt, 6, order.getPrice());
    sqlite3_bind_int(stmt, 7, static_cast<int>(order.getType()));
    sqlite3_bind_int(stmt, 8, static_cast<int>(order.getStatus()));

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);

    if (!success)
    {
        std::cerr << "Failed to save order: "
                  << sqlite3_errmsg(db)
                  << std::endl;
    }

    sqlite3_finalize(stmt);

    return success;
}

//loading the orders from the database
bool Database::loadOrders(std::vector<Order>& orders)
{
    if(!open()){
        return false;
    }

    orders.clear();

    const char* sql="SELECT * FROM orders WHERE status!=?";

    sqlite3_stmt* stmt=nullptr;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK){
        std::cerr<<"Failed to prepare LoadOrders statement: "
        <<std::endl;
        return false;
    }

     sqlite3_bind_int(stmt, 1, static_cast<int>(OrderStatus::CANCELLED));

    while(sqlite3_step(stmt)==SQLITE_ROW)
    {
       
        int orderId=sqlite3_column_int(stmt, 0);
        int userId=sqlite3_column_int(stmt, 1);


        std::string stockSymbol=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));


        int quantity=sqlite3_column_int(stmt, 3);

        int remainingQuantity=sqlite3_column_int(stmt, 4);

        double price=sqlite3_column_double(stmt, 5);

        OrderType type=static_cast<OrderType>(sqlite3_column_int(stmt, 6));
        OrderStatus status=static_cast<OrderStatus>(sqlite3_column_int(stmt, 7));

        Order order(
            orderId,
            userId, 
            stockSymbol, 
            quantity, 
            price, 
            type
        );
        order.setRemainingQuantity(remainingQuantity);
        order.setStatus(status);

        orders.push_back(order);
    }
    sqlite3_finalize(stmt);
    return true;
    
}

//updating the order status in the database
bool Database::updateOrderStatus(
    int orderId,
    OrderStatus status,
    int remainingQuantity
)
{

    if(!orderExists(orderId))
    {
        return false;
    }


    const char* sql=
        "UPDATE orders "
        "SET status=?, remainingQuantity=? "
        "WHERE orderId=?;";

    sqlite3_stmt* stmt=nullptr;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK)
    {
        std::cerr<<"Failed to prepare UpdateOrderStatus statement: "
        <<sqlite3_errmsg(db)
        <<std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, static_cast<int>(status));
    sqlite3_bind_int(stmt, 2, remainingQuantity);
    sqlite3_bind_int(stmt, 3, orderId);

    bool success= (sqlite3_step(stmt)==SQLITE_DONE);

    if(!success)
    {
        std::cerr<<"Failed to update order status: "
        <<sqlite3_errmsg(db)
        <<std::endl;
    }
    sqlite3_finalize(stmt);
    return success;
}


bool Database::cancelOrder(int orderId)
{   
    if(!orderExists(orderId))
    {
        return false;
    }
    const char* sql=
        "UPDATE orders "
        "SET status=?, remainingQuantity=? "
        "WHERE orderId=?;";

    sqlite3_stmt* stmt=nullptr;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK)
    {
        std::cerr<<"Failed to prepare CancelOrder statement: "
        <<sqlite3_errmsg(db)
        <<std::endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, static_cast<int>(OrderStatus::CANCELLED));
    sqlite3_bind_int(stmt, 2, 0); // Set remainingQuantity to 0
    sqlite3_bind_int(stmt, 3, orderId);

    bool success= (sqlite3_step(stmt)==SQLITE_DONE);
    if(!success)
    {
        std::cerr<<"Failed to cancel order: "
        <<sqlite3_errmsg(db)
        <<std::endl;
    }
    sqlite3_finalize(stmt);
    return success;
}

bool Database::updatePortfolio(int clientId, const std::string& symbol, int quantity, double averagePrice)
{
    if(!open())
    {
        return false;
    }

    const char* sql=
    "INSERT OR REPLACE INTO portfolio(clientId, symbol, quantity, averagePrice)"
    "VALUES(?,?,?,?)";

    sqlite3_stmt* stmt=nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK)
    {
        return false;
    }

    sqlite3_bind_int(stmt, 1, clientId);
    sqlite3_bind_text(stmt, 2, symbol.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, quantity);
    sqlite3_bind_double(stmt, 4, averagePrice);

    bool success=sqlite3_step(stmt)==SQLITE_DONE;

    sqlite3_finalize(stmt);
    return success;

}

bool Database::loadPortfolio(int clientId, std::vector<std::pair<std::string,int>>& holdings)
{
    if(!open())
    {
        return false;
    }

    holdings.clear();

    const char* sql="SELECT symbol, quantity FROM portfolio WHERE clientId=?";

    sqlite3_stmt* stmt=nullptr;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK)
    {

        std::cerr<<"Failed to prepare LoadPortfolio statement: "
        <<std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, clientId);

    while(sqlite3_step(stmt)==SQLITE_ROW)
    {
        std::string symbol=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int quantity=sqlite3_column_int(stmt, 1);

        holdings.push_back({symbol, quantity});
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::loadPortfolioDetailed(int clientId, std::vector<std::tuple<std::string, int, double>>& holdings)
{
    if(!open())
    {
        return false;
    }

    holdings.clear();

    const char* sql="SELECT symbol, quantity, averagePrice FROM portfolio WHERE clientId=?";

    sqlite3_stmt* stmt=nullptr;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK)
    {
        std::cerr<<"Failed to prepare LoadPortfolioDetailed statement: "
        <<std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, clientId);

    while(sqlite3_step(stmt)==SQLITE_ROW)
    {
        std::string symbol=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int quantity=sqlite3_column_int(stmt, 1);
        double averagePrice=sqlite3_column_double(stmt, 2);

        holdings.emplace_back(symbol, quantity, averagePrice);
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::updateCollateral(int clientId, double available, double blocked, double total)
{
    if(!open())
    {
        return false;
    }

    (void)total; // Suppress unused variable warning.....calculated not stored in the database

    const char* sql=
    "INSERT OR REPLACE INTO collateral(clientId, availableBalance, blockedBalance)"
    "VALUES(?,?,?)";

    sqlite3_stmt* stmt=nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK)
    {
        std::cerr<<"Failed to prepare UpdateCollateral statement: "
        <<std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, clientId);
    sqlite3_bind_double(stmt, 2, available);
    sqlite3_bind_double(stmt, 3, blocked);

    bool success=sqlite3_step(stmt)==SQLITE_DONE;

    sqlite3_finalize(stmt);
    return success;
}


bool Database::loadCollateral(int clientId, double& available, double& blocked, double& total)
{
    if(!open())
    {
        return false;
    }

    const char* sql="SELECT availableBalance, blockedBalance FROM collateral WHERE clientId=?";

    sqlite3_stmt* stmt=nullptr;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK)
    {
        std::cerr<<"Failed to prepare LoadCollateral statement: "
        <<std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, clientId);

    bool success=false;

    if(sqlite3_step(stmt)==SQLITE_ROW)
    {
        available=sqlite3_column_double(stmt, 0);
        blocked=sqlite3_column_double(stmt, 1);
        total=available+blocked;
        success=true;
    }

    sqlite3_finalize(stmt);
    return success;
}

bool Database::saveTransaction(
    int buyerId,
    int sellerId,
    const Order& order,
    int quantity,
    double executionPrice
)
{
    if(!userExists(buyerId))
    {
        return false;
    }
    if(!userExists(sellerId))
    {
        return false;
    }

    const char* sql=
    "INSERT INTO transactions(buyerId, sellerId, symbol, quantity, price)"
    "VALUES(?,?,?,?,?)";

    sqlite3_stmt* stmt=nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)!=SQLITE_OK)
    {
        std::cerr<<"Failed to prepare SaveTransaction statement: "
        <<std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, buyerId);
    sqlite3_bind_int(stmt, 2, sellerId);
    sqlite3_bind_text(stmt, 3, order.getStockSymbol().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, quantity);
    sqlite3_bind_double(stmt, 5, executionPrice);

    bool success=sqlite3_step(stmt)==SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}
