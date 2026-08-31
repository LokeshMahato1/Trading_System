    # Database Module

## 1. Overview

The **Database Module** is the persistence layer of the Trading Management System (TMS).

It provides a centralized interface between the trading engine and the SQLite database. The module is responsible for storing, retrieving, updating, and managing persistent TMS data.

The `Database` class provides operations for:

* Database connection management
* Table creation
* Stock storage and retrieval
* User authentication and management
* Order storage and retrieval
* Portfolio management
* Collateral management
* Transaction/trade history
* Password updates
* Order status updates
* Order cancellation

The module uses **SQLite3** as the database engine.

---

# 2. Module Responsibilities

The `Database` class is responsible for:

1. Opening the SQLite database.
2. Closing the database safely.
3. Creating required database tables.
4. Storing stock information.
5. Loading stock information.
6. Verifying user login credentials.
7. Adding users.
8. Removing users.
9. Loading users.
10. Checking whether users exist.
11. Saving orders.
12. Loading orders.
13. Loading client portfolio information.
14. Updating portfolio holdings.
15. Loading client collateral.
16. Updating collateral.
17. Saving completed trade transactions.
18. Changing user passwords.
19. Updating order status.
20. Cancelling orders.

---

# 3. File Structure

The database module is organized as:

```text
database/
├── db.hpp
└── db.cpp
```

The header file contains the public interface and private members.

The `.cpp` file contains the actual SQLite database implementation.

---

# 4. Dependencies

The database header includes:

```cpp
#include <string>
#include <tuple>
#include <vector>
#include <sqlite3.h>
```

It also depends on TMS models:

```cpp
#include "../model/models.hpp"
#include "../order/order.h"
```

### Standard C++ Libraries

| Library    | Purpose                                             |
| ---------- | --------------------------------------------------- |
| `<string>` | String values such as usernames, symbols, and roles |
| `<tuple>`  | Returning multiple database fields together         |
| `<vector>` | Collections of stocks, users, orders, etc.          |

### External Library

```cpp
#include <sqlite3.h>
```

SQLite3 provides the embedded relational database used by the TMS.

---

# 5. Database Architecture

The database module sits between the TMS engine and SQLite.

```text
                    Trading Management System
                              |
                              v
                       +-------------+
                       |  Database   |
                       |    Class    |
                       +------+------+
                              |
                              v
                         SQLite3 DB
                              |
          +-------------------+-------------------+
          |         |         |        |          |
          v         v         v        v          v
       Users     Stocks    Orders  Portfolio  Collateral
                              |
                              v
                         Transactions
```

Other modules should interact with the database through the `Database` class instead of directly calling SQLite functions.

---

# 6. Database Class

The main class is:

```cpp
class Database
{
private:
    sqlite3* db;
    const std::string dbName;

    bool executeSQL(const std::string& sql);

    bool userExists(int userId);
    bool stockExists(const std::string& symbol);
    bool orderExists(int orderId);

public:
    Database();
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool open();
    bool close();

    bool createTables();

    bool addStocks(const std::vector<StockData>& stocks);
    bool loadStocks(std::vector<StockData>& stocks);

    bool verifyUser(
        const std::string& username,
        const std::string& password,
        std::string& role,
        int& userId
    );

    bool addUser(
        const std::string& name,
        const std::string& username,
        const std::string& password,
        const std::string& role
    );

    bool removeUser(int userId);

    bool loadUsers(
        std::vector<std::tuple<int, std::string, std::string, std::string>>& users
    );

    bool hasAnyUsers();

    bool saveOrder(const Order& order);
    bool loadOrders(std::vector<Order>& orders);

    bool loadPortfolio(
        int clientId,
        std::vector<std::pair<std::string,int>>& holdings
    );

    bool loadPortfolioDetailed(
        int clientId,
        std::vector<std::tuple<std::string, int, double>>& holdings
    );

    bool updatePortfolio(
        int clientId,
        const std::string& symbol,
        int quantity,
        double averagePrice
    );

    bool loadCollateral(
        int clientId,
        double& available,
        double& blocked,
        double& total
    );

    bool updateCollateral(
        int clientId,
        double available,
        double blocked,
        double total
    );

    bool saveTransaction(
        int buyerId,
        int sellerId,
        const Order& order,
        int quantity,
        double executionPrice
    );

    bool changePassword(
        int userId,
        const std::string& newPassword
    );

    bool updateOrderStatus(
        int orderId,
        OrderStatus status,
        int remainingQuantity
    );

    bool cancelOrder(int orderId);
};
```

---

# 7. Internal Database Handle

The class contains:

```cpp
sqlite3* db;
```

This pointer represents the SQLite database connection.

Conceptually:

```text
Database object
      |
      v
sqlite3*
      |
      v
SQLite database
```

The pointer is used internally by the implementation to execute SQL commands and queries.

---

# 8. Database Name

The class also contains:

```cpp
const std::string dbName;
```

This stores the name/path of the database file.

For example:

```text
sqlite.db
```

The `const` qualifier means that the database name cannot be changed after the `Database` object is constructed.

---

# 9. Constructor

```cpp
Database();
```

The constructor creates a `Database` object.

Its responsibility is to initialize the database-related members and prepare the object for database operations.

Typical lifecycle:

```text
Database object created
        |
        v
Database connection opened
        |
        v
Tables created
        |
        v
Database operations
        |
        v
Connection closed
        |
        v
Database object destroyed
```

---

# 10. Destructor

```cpp
~Database();
```

The destructor is responsible for cleaning up the database object.

A correctly implemented destructor should ensure that an open SQLite connection is closed before the object is destroyed.

This prevents database resources from being left open.

---

# 11. Copy Prevention

The class explicitly disables copying:

```cpp
Database(const Database&) = delete;
Database& operator=(const Database&) = delete;
```

This is important because a `Database` object owns a SQLite database connection.

Allowing objects to be copied could result in multiple objects incorrectly managing the same database connection.

Therefore:

```cpp
Database db1;

Database db2 = db1;   // Not allowed
```

is prevented by the compiler.

---

# 12. Database Connection Management

The class provides:

```cpp
bool open();
bool close();
```

These functions manage the SQLite connection.

### `open()`

Opens the database.

```text
Database file
     |
     v
SQLite connection
     |
     v
sqlite3*
```

### `close()`

Closes the SQLite connection and releases the associated database resources.

---

# 13. `executeSQL()`

```cpp
bool executeSQL(const std::string& sql);
```

This is a private helper function.

Its purpose is to execute SQL statements that do not require returning rows.

Examples include:

```sql
CREATE TABLE ...
```

or:

```sql
UPDATE ...
```

or:

```sql
DELETE ...
```

Keeping SQL execution in a private helper reduces duplicated SQLite error-handling code.

---

# 14. Existence Checking

The class provides three private helper functions:

```cpp
bool userExists(int userId);
bool stockExists(const std::string& symbol);
bool orderExists(int orderId);
```

These functions determine whether a particular record already exists.

---

## 14.1 `userExists()`

```cpp
bool userExists(int userId);
```

Checks whether a user with the specified ID exists.

Used when performing operations that require a valid user.

---

## 14.2 `stockExists()`

```cpp
bool stockExists(const std::string& symbol);
```

Checks whether a stock exists in the database.

The stock symbol is used as the identifier.

Example:

```text
AAPL
MSFT
NVDA
```

---

## 14.3 `orderExists()`

```cpp
bool orderExists(int orderId);
```

Checks whether an order with a particular ID exists.

This is useful before performing operations such as:

* Updating an order
* Cancelling an order
* Modifying an order

---

# 15. Table Creation

## Function

```cpp
bool createTables();
```

This function creates the database tables required by the TMS.

It is normally called during system initialization.

Workflow:

```text
TradingSystem
      |
      v
db.createTables()
      |
      v
Create required tables
      |
      v
Database ready
```

If table creation fails, the trading system can stop initialization.

---

# 16. Stock Operations

The database provides two stock-related functions:

```cpp
bool addStocks(
    const std::vector<StockData>& stocks
);

bool loadStocks(
    std::vector<StockData>& stocks
);
```

---

## 16.1 Adding Stocks

```cpp
bool addStocks(
    const std::vector<StockData>& stocks
);
```

Stores stock information in the database.

The function accepts:

```cpp
std::vector<StockData>
```

meaning multiple stocks can be inserted.

Example:

```text
StockData
   |
   +-- AAPL
   +-- MSFT
   +-- GOOGL
   +-- AMZN
   +-- TSLA
```

---

## 16.2 Loading Stocks

```cpp
bool loadStocks(
    std::vector<StockData>& stocks
);
```

Retrieves stock information from the database and stores the results in the provided vector.

Workflow:

```text
SQLite
  |
  v
Stock table
  |
  v
loadStocks()
  |
  v
vector<StockData>
  |
  v
Market
```

---

# 17. User Management

The database provides several user-related operations:

```cpp
verifyUser()
addUser()
removeUser()
loadUsers()
hasAnyUsers()
changePassword()
```

These functions support authentication and user administration.

---

# 18. User Authentication

## Function

```cpp
bool verifyUser(
    const std::string& username,
    const std::string& password,
    std::string& role,
    int& userId
);
```

This function verifies the credentials provided during login.

### Input

```text
Username
Password
```

### Output

If authentication succeeds, the function provides:

```text
Role
User ID
```

through reference parameters.

Workflow:

```text
Username + Password
        |
        v
   verifyUser()
        |
        v
    Database
        |
        v
Credentials Match?
     /       \
   Yes        No
   |           |
   v           v
User ID      false
Role
```

---

# 19. Adding Users

## Function

```cpp
bool addUser(
    const std::string& name,
    const std::string& username,
    const std::string& password,
    const std::string& role
);
```

Creates a new user record.

The information includes:

```text
Name
Username
Password
Role
```

Possible roles in the TMS include:

```text
ADMIN
CLIENT
```

The administrator session can use this function to create new users.

---

# 20. Removing Users

```cpp
bool removeUser(int userId);
```

Removes a user identified by their ID.

The function can first verify that the user exists using:

```cpp
userExists(userId);
```

and then perform the corresponding database operation.

---

# 21. Loading Users

```cpp
bool loadUsers(
    std::vector<
        std::tuple<
            int,
            std::string,
            std::string,
            std::string
        >
    >& users
);
```

Loads user records from the database.

Each tuple contains four values:

```text
User ID
Name
Username
Role
```

Conceptually:

```text
vector
 |
 +-- tuple(ID, Name, Username, Role)
 |
 +-- tuple(ID, Name, Username, Role)
 |
 +-- tuple(ID, Name, Username, Role)
```

---

# 22. Checking for Existing Users

```cpp
bool hasAnyUsers();
```

Determines whether the database contains at least one user.

This is especially useful during first-time system startup.

The system can use:

```text
No users
   |
   v
Create first administrator
```

This supports the administrator bootstrap process.

---

# 23. Order Management

The database provides:

```cpp
bool saveOrder(const Order& order);
bool loadOrders(std::vector<Order>& orders);
bool updateOrderStatus(
    int orderId,
    OrderStatus status,
    int remainingQuantity
);
bool cancelOrder(int orderId);
```

These functions provide persistent order management.

---

# 24. Saving an Order

```cpp
bool saveOrder(const Order& order);
```

Stores an `Order` object in the database.

The order may contain information such as:

```text
Order ID
User ID
Stock Symbol
Quantity
Remaining Quantity
Price
Order Type
Order Status
```

The exact fields depend on the `Order` model.

Workflow:

```text
Client
  |
  v
Create Order
  |
  v
saveOrder()
  |
  v
SQLite Orders Table
```

---

# 25. Loading Orders

```cpp
bool loadOrders(
    std::vector<Order>& orders
);
```

Retrieves orders from the database.

The matching engine uses this function to reconstruct the active order book.

Workflow:

```text
SQLite Orders
      |
      v
loadOrders()
      |
      v
vector<Order>
      |
      v
OrderBook
      |
      v
MatchMaking
```

---

# 26. Updating Order Status

```cpp
bool updateOrderStatus(
    int orderId,
    OrderStatus status,
    int remainingQuantity
);
```

Updates an existing order's:

* Status
* Remaining quantity

For example, an order may transition through:

```text
PENDING
   |
   v
PARTIALLY_FILLED
   |
   v
FILLED
```

Other possible states include:

```text
CANCELLED
FAILED
```

---

# 27. Cancelling an Order

```cpp
bool cancelOrder(int orderId);
```

Cancels an existing order using its order ID.

The operation is persistent, meaning the cancellation is recorded in the database rather than existing only in memory.

Workflow:

```text
Client
  |
  v
Cancel Order
  |
  v
cancelOrder(orderId)
  |
  v
Database
  |
  v
Order Status Updated
```

---

# 28. Portfolio Management

The database provides:

```cpp
bool loadPortfolio(
    int clientId,
    std::vector<std::pair<std::string,int>>& holdings
);
```

and:

```cpp
bool loadPortfolioDetailed(
    int clientId,
    std::vector<std::tuple<std::string, int, double>>& holdings
);
```

and:

```cpp
bool updatePortfolio(
    int clientId,
    const std::string& symbol,
    int quantity,
    double averagePrice
);
```

---

# 29. Loading Basic Portfolio Information

```cpp
bool loadPortfolio(
    int clientId,
    std::vector<std::pair<std::string,int>>& holdings
);
```

Loads the client's holdings.

Each pair contains:

```text
Stock Symbol
Quantity
```

Example:

```text
AAPL  -> 100
MSFT  -> 50
NVDA  -> 25
```

---

# 30. Loading Detailed Portfolio Information

```cpp
bool loadPortfolioDetailed(
    int clientId,
    std::vector<
        std::tuple<std::string, int, double>
    >& holdings
);
```

Provides more detailed portfolio information.

Each tuple contains:

```text
Symbol
Quantity
Average Price
```

Example:

```text
AAPL
100
175.50
```

This detailed version is particularly useful when restoring a client's portfolio after application startup.

---

# 31. Updating Portfolio

```cpp
bool updatePortfolio(
    int clientId,
    const std::string& symbol,
    int quantity,
    double averagePrice
);
```

Updates a client's holding for a particular stock.

The information stored includes:

```text
Client ID
Stock Symbol
Quantity
Average Price
```

Workflow:

```text
Trade Execution
      |
      v
Portfolio Updated
      |
      v
updatePortfolio()
      |
      v
SQLite
```

---

# 32. Collateral Management

The database provides:

```cpp
bool loadCollateral(
    int clientId,
    double& available,
    double& blocked,
    double& total
);
```

and:

```cpp
bool updateCollateral(
    int clientId,
    double available,
    double blocked,
    double total
);
```

These functions persist client collateral information.

---

# 33. Loading Collateral

```cpp
bool loadCollateral(
    int clientId,
    double& available,
    double& blocked,
    double& total
);
```

Retrieves three collateral values:

```text
Available Collateral
Blocked Collateral
Total Collateral
```

Conceptually:

```text
                 Client Collateral
                       |
          +------------+------------+
          |            |            |
          v            v            v
      Available     Blocked       Total
```

---

# 34. Updating Collateral

```cpp
bool updateCollateral(
    int clientId,
    double available,
    double blocked,
    double total
);
```

Stores the current collateral state in the database.

This is important after operations such as:

* Depositing collateral
* Withdrawing collateral
* Reserving collateral
* Releasing collateral
* Deducting collateral
* Executing trades

---

# 35. Transaction Management

The database provides:

```cpp
bool saveTransaction(
    int buyerId,
    int sellerId,
    const Order& order,
    int quantity,
    double executionPrice
);
```

This records a completed trade transaction.

---

# 36. Transaction Information

The function receives:

```text
Buyer ID
Seller ID
Order
Executed Quantity
Execution Price
```

The transaction can therefore represent the result of a matched and executed trade.

Workflow:

```text
Buy Order
    |
    +----------------+
                     |
                     v
               MatchMaking
                     |
                     v
              Matched Orders
                     |
                     v
              TradeExecution
                     |
                     v
             saveTransaction()
                     |
                     v
              Trade History
```

---

# 37. Password Management

## Function

```cpp
bool changePassword(
    int userId,
    const std::string& newPassword
);
```

Changes the password associated with a user ID.

Workflow:

```text
Admin / User
      |
      v
New Password
      |
      v
changePassword()
      |
      v
Users Table
```

The function returns `true` if the operation succeeds and `false` if it fails.

---

# 38. Database and Trading Engine Integration

The database is heavily integrated with the trading engine.

A typical trade workflow is:

```text
              Client A
                 |
                 v
            Buy Order
                 |
                 v
              Database
                 |
                 v
             OrderBook
                 |
                 v
             MatchMaking
                 |
                 v
              Matched
                 |
                 v
          TradeExecution
                 |
        +--------+--------+
        |        |        |
        v        v        v
    Portfolio Collateral Transaction
        |        |        |
        +--------+--------+
                 |
                 v
              Database
```

This allows the in-memory trading state and persistent database state to remain synchronized.

---

# 39. Database Lifecycle

The normal lifecycle is:

```text
             Application Start
                    |
                    v
             Database()
                    |
                    v
                  open()
                    |
                    v
             createTables()
                    |
                    v
            Database Operations
                    |
       +------------+------------+
       |            |            |
       v            v            v
     Users        Orders       Trades
       |            |            |
       +------------+------------+
                    |
                    v
                 close()
                    |
                    v
                  ~Database()
```

---

# 40. Data Flow

The database serves as the persistent source of system state.

```text
                  +----------------+
                  |    Database    |
                  +-------+--------+
                          |
       +------------------+------------------+
       |          |         |        |       |
       v          v         v        v       v
     Users      Stocks    Orders Portfolio Collateral
       |          |         |        |       |
       |          |         |        |       |
       +----------+---------+--------+-------+
                          |
                          v
                    Trading Engine
                          |
                          v
                     Transactions
                          |
                          v
                       Database
```

---

# 41. Error Handling

All database operations return:

```cpp
bool
```

This provides a simple success/failure interface.

For example:

```cpp
if (!db.saveOrder(order))
{
    // Handle failure
}
```

The same pattern can be used for:

```cpp
open()
close()
createTables()
addStocks()
loadStocks()
addUser()
removeUser()
saveOrder()
loadOrders()
updatePortfolio()
updateCollateral()
saveTransaction()
```

This makes database failures easy for higher-level modules such as `TradingSystem` to detect.

---

# 42. Why SQLite Is Used

SQLite is suitable for the TMS because it is:

* Embedded
* Lightweight
* Serverless
* Easy to deploy
* File-based
* Appropriate for a desktop application

The TMS therefore does not need a separate database server for basic operation.

The database can be represented as:

```text
Trading Management System
          |
          v
       SQLite
          |
          v
      sqlite.db
```

---

# 43. Persistence vs In-Memory State

The TMS contains both in-memory objects and persistent database records.

For example:

```text
             In Memory
          +-------------+
          |  Portfolio  |
          |  Collateral |
          |  OrderBook  |
          +------+------+
                 |
                 | Synchronization
                 v
          +-------------+
          |   SQLite    |
          |   Database  |
          +-------------+
```

The database allows information to survive application restarts.

Without persistence:

```text
Application closes
        |
        v
In-memory data lost
```

With the database:

```text
Application closes
        |
        v
Data stored in SQLite
        |
        v
Application restarts
        |
        v
Data loaded again
```

---

# 44. Client State Restoration

The `TradingSystem` can restore a client's state using:

```cpp
db.loadPortfolioDetailed(...)
```

and:

```cpp
db.loadCollateral(...)
```

The restored information is then transferred to:

```text
Portfolio
Collateral
```

This produces:

```text
SQLite
  |
  +---- Portfolio ------> Portfolio Object
  |
  +---- Collateral -----> Collateral Object
```

This is important for maintaining continuity between application sessions.

---

# 45. Matching Engine Database Workflow

The matching engine uses the database to reconstruct active orders.

```text
SQLite Orders
      |
      v
db.loadOrders()
      |
      v
Order Vector
      |
      v
Filter Pending /
Partially Filled
      |
      v
OrderBook
      |
      v
MatchMaking
      |
      v
TradeExecution
```

After execution, the system updates the database with:

```text
Order status
Remaining quantity
Portfolio
Collateral
Transaction
```

---

# 46. Relationship With Other Modules

| Module           | Database Interaction                                    |
| ---------------- | ------------------------------------------------------- |
| `TradingSystem`  | Initialization, authentication, state restoration       |
| `Market`         | Loads stock data                                        |
| `Client`         | Saves orders and manages client data                    |
| `Admin`          | User management                                         |
| `OrderBook`      | Receives active orders loaded from DB                   |
| `MatchMaking`    | Uses orders retrieved from DB indirectly                |
| `TradeExecution` | Updates portfolio, collateral, transactions, and orders |
| `Portfolio`      | State loaded and persisted through DB                   |
| `Collateral`     | State loaded and persisted through DB                   |

---

# 47. Security Considerations

The database contains potentially sensitive information such as:

* User credentials
* User information
* Orders
* Trading history
* Portfolio information
* Collateral information

Therefore, database operations should be implemented carefully.

Important considerations include:

### SQL Injection Protection

User-provided values should preferably be inserted using SQLite prepared statements rather than constructing SQL queries through direct string concatenation.

### Password Protection

Passwords should not be stored as plain text.

The authentication system should use a secure password hashing mechanism.

### Database Access

The SQLite database file should not be unnecessarily exposed or committed to a public repository.

For example:

```gitignore
sqlite.db
*.db
```

can be considered for local database files, depending on the project's deployment requirements.

---

# 48. Public API Reference

## Connection

| Function  | Return Type | Purpose                |
| --------- | ----------- | ---------------------- |
| `open()`  | `bool`      | Opens SQLite database  |
| `close()` | `bool`      | Closes SQLite database |

## Database Setup

| Function         | Return Type | Purpose                 |
| ---------------- | ----------- | ----------------------- |
| `createTables()` | `bool`      | Creates required tables |

## Stocks

| Function       | Return Type | Purpose             |
| -------------- | ----------- | ------------------- |
| `addStocks()`  | `bool`      | Adds stock records  |
| `loadStocks()` | `bool`      | Loads stock records |

## Users

| Function           | Return Type | Purpose                    |
| ------------------ | ----------- | -------------------------- |
| `verifyUser()`     | `bool`      | Authenticates a user       |
| `addUser()`        | `bool`      | Adds a user                |
| `removeUser()`     | `bool`      | Removes a user             |
| `loadUsers()`      | `bool`      | Loads users                |
| `hasAnyUsers()`    | `bool`      | Checks whether users exist |
| `changePassword()` | `bool`      | Changes a password         |

## Orders

| Function              | Return Type | Purpose              |
| --------------------- | ----------- | -------------------- |
| `saveOrder()`         | `bool`      | Saves an order       |
| `loadOrders()`        | `bool`      | Loads orders         |
| `updateOrderStatus()` | `bool`      | Updates order status |
| `cancelOrder()`       | `bool`      | Cancels an order     |

## Portfolio

| Function                  | Return Type | Purpose                            |
| ------------------------- | ----------- | ---------------------------------- |
| `loadPortfolio()`         | `bool`      | Loads basic holdings               |
| `loadPortfolioDetailed()` | `bool`      | Loads holdings with average prices |
| `updatePortfolio()`       | `bool`      | Updates a holding                  |

## Collateral

| Function             | Return Type | Purpose                  |
| -------------------- | ----------- | ------------------------ |
| `loadCollateral()`   | `bool`      | Loads collateral state   |
| `updateCollateral()` | `bool`      | Updates collateral state |

## Transactions

| Function            | Return Type | Purpose                           |
| ------------------- | ----------- | --------------------------------- |
| `saveTransaction()` | `bool`      | Saves completed trade information |

---

# 49. Private API Reference

| Function        | Purpose                            |
| --------------- | ---------------------------------- |
| `executeSQL()`  | Executes SQL statements internally |
| `userExists()`  | Checks whether a user exists       |
| `stockExists()` | Checks whether a stock exists      |
| `orderExists()` | Checks whether an order exists     |

These functions are private because they are implementation details and should not normally be called directly by other modules.

---

# 50. Overall Database Workflow

The complete database workflow can be summarized as:

```text
                         Database Module
                                |
        +-----------------------+-----------------------+
        |                       |                       |
        v                       v                       v
   Connection              Data Storage            Data Retrieval
        |                       |                       |
        v                       v                       v
      open()                 Users                  Users
      close()                Stocks                 Stocks
                             Orders                 Orders
                             Portfolio              Portfolio
                             Collateral             Collateral
                             Transactions           Transactions
                                |
                                v
                         Data Updates
                                |
             +------------------+------------------+
             |                  |                  |
             v                  v                  v
        Order Status        Portfolio         Collateral
             |                  |                  |
             +------------------+------------------+
                                |
                                v
                             SQLite
```

---

# 51. Summary

The **Database Module** is the persistent storage layer of the Trading Management System.

It provides a clean interface between the C++ trading engine and SQLite.

Its major responsibilities are:

```text
Database
   |
   +-- Connection Management
   |
   +-- Table Creation
   |
   +-- User Management
   |
   +-- Stock Management
   |
   +-- Order Management
   |
   +-- Portfolio Management
   |
   +-- Collateral Management
   |
   +-- Transaction Management
   |
   +-- Password Management
```

The most important design principle is that other TMS modules do not need to know how SQLite works internally. They simply call functions such as:

```cpp
db.saveOrder(order);
db.loadOrders(orders);

db.updatePortfolio(...);
db.loadPortfolioDetailed(...);

db.updateCollateral(...);
db.loadCollateral(...);

db.saveTransaction(...);

db.verifyUser(...);
```

Therefore, the `Database` class acts as an abstraction layer:

```text
+------------------------------+
|       Trading Engine         |
+---------------+--------------+
                |
                | Database API
                v
+------------------------------+
|          Database            |
|            Class             |
+---------------+--------------+
                |
                | SQLite API
                v
+------------------------------+
|          SQLite3             |
|         sqlite.db            |
+------------------------------+
```

This separation makes the TMS architecture more modular, maintainable, and easier to test.
