# Trading System Module

## 1. Overview

The **Trading System Module** is the central orchestration component of the Trading Management System (TMS).

The `TradingSystem` class connects the major components of the system and controls the overall application workflow.

It coordinates:

* Database management
* Market data
* Order book management
* Order matching
* Trade execution
* User authentication
* Client sessions
* Admin sessions
* Client portfolio and collateral state
* System initialization
* System shutdown

The module acts as the main controller between the user interface/console and the underlying trading engine.

---

# 2. Module Responsibilities

The `TradingSystem` class is responsible for:

1. Creating and managing the core TMS components.
2. Initializing database tables.
3. Loading market/stock data.
4. Creating the first administrator when no users exist.
5. Authenticating users.
6. Creating the appropriate `Admin` or `Client` object after authentication.
7. Running the admin session.
8. Running the client session.
9. Loading client portfolio and collateral information from the database.
10. Rebuilding the order book from database orders.
11. Running the matchmaking engine.
12. Executing matched trades.
13. Handling logout.
14. Shutting down the system cleanly.

---

# 3. Architecture Position

`TradingSystem` sits at the center of the application and coordinates the major TMS modules.

```text
                         +----------------+
                         |     Client     |
                         +-------+--------+
                                 |
                                 v
                         +----------------+
                         | TradingSystem  |
                         +-------+--------+
                                 |
          +----------------------+----------------------+
          |          |            |          |           |
          v          v            v          v           v
      Database     Market     OrderBook  MatchMaking  TradeExecution
          |          |            |          |           |
          |          |            |          |           |
          +----------+------------+----------+-----------+
                                 |
                                 v
                         +----------------+
                         | Portfolio /    |
                         | Collateral     |
                         +----------------+
```

The `TradingSystem` does not itself implement the detailed trading algorithms. Instead, it coordinates the specialized modules.

---

# 4. File Structure

The module is organized around:

```text
TradingSystem/
├── tradingSystem.h
└── tradingSystem.cpp
```

The implementation also includes other TMS modules:

```cpp
#include "../market/market.h"
#include "../execution/tradeExecution.h"
#include "../Matchmaking/matchmaking.h"
#include "../orderbook/orderBook.hpp"
#include "../users/user.h"
#include "../database/db.hpp"
```

The implementation additionally uses:

```cpp
#include "../users/admin.h"
#include "../users/client.h"
#include "../util/consoleInput.hpp"
```

These dependencies allow the system controller to communicate with users, trading components, and the database.

---

# 5. Class Definition

The main class is:

```cpp
class TradingSystem {
private:
    Database db;
    Market market;
    TradeExecution tradeExecution;
    MatchMaking matchmaking;
    OrderBook orderBook;

    std::unique_ptr<User> currentUser;

    bool loadClientState(
        int clientId,
        Portfolio& portfolio,
        Collateral& collateral
    );

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
```

---

# 6. Core Components

The `TradingSystem` contains instances of the major engine components.

## 6.1 Database

```cpp
Database db;
```

The database object manages persistent system data.

It is used for:

* Creating database tables
* Loading users
* Loading orders
* Verifying users
* Loading portfolios
* Loading collateral
* Updating order status
* Closing the database

---

## 6.2 Market

```cpp
Market market;
```

The `Market` object manages stock/market data.

The trading system uses it during initialization to load stock information.

```cpp
if(!market.loadStocks())
{
    ...
}
```

The system continues running even if stock data cannot be loaded, although a warning is displayed.

---

## 6.3 Trade Execution

```cpp
TradeExecution tradeExecution;
```

The `TradeExecution` object is responsible for executing trades after matching.

It is initialized with the database:

```cpp
tradeExecution(db)
```

This allows trade execution to synchronize the resulting state with persistent storage.

---

## 6.4 MatchMaking

```cpp
MatchMaking matchmaking;
```

The matchmaking component determines whether buy and sell orders can be matched.

The `TradingSystem` repeatedly calls:

```cpp
matchmaking.matchOrders(...)
```

until no executable match remains.

---

## 6.5 OrderBook

```cpp
OrderBook orderBook;
```

The order book stores active buy and sell orders.

During matching-engine execution, the order book is reconstructed from database orders.

---

# 7. Current User Management

The system stores the currently authenticated user using:

```cpp
std::unique_ptr<User> currentUser;
```

Using `std::unique_ptr` provides automatic memory management.

The pointer can refer to either:

```text
User
 |
 +-- Admin
 |
 +-- Client
```

After successful authentication, the appropriate derived class is dynamically created.

For an administrator:

```cpp
currentUser = std::make_unique<Admin>(...);
```

For a client:

```cpp
currentUser = std::make_unique<Client>(...);
```

The implementation then uses the common `User` interface to manage the active session.

---

# 8. Constructor

```cpp
TradingSystem::TradingSystem()
    : db(),
      market(),
      tradeExecution(db),
      matchmaking()
{}
```

The constructor initializes the main components.

Initialization order is:

```text
Database
   |
   v
Market
   |
   v
TradeExecution(Database)
   |
   v
MatchMaking
```

The `TradeExecution` object receives the database object:

```cpp
tradeExecution(db)
```

so it can interact with persistent trading data.

---

# 9. System Initialization

## Function

```cpp
bool TradingSystem::initializeSystem()
```

This function prepares the TMS for operation.

The initialization sequence is:

```text
             Start
               |
               v
      Initialize TradingSystem
               |
               v
       Create Database Tables
               |
               v
       Check First Administrator
               |
               v
         Load Stock Data
               |
               v
       System Ready
```

The implementation prints:

```text
Initializing Trading System...
```

and then creates the required database tables.

---

# 10. Database Initialization

The first major initialization operation is:

```cpp
if(!db.createTables())
{
    std::cerr << "Failed to create database tables."
              << std::endl;
    return false;
}
```

If table creation fails, initialization stops.

The function returns:

```cpp
false
```

If successful:

```text
Database tables created successfully.
```

is displayed.

---

# 11. First Administrator Bootstrap

After creating the database tables, the system calls:

```cpp
bootstrapFirstAdmin(db);
```

The purpose of this function is to ensure that a newly created database has an administrator account.

The process is:

```text
Check database for users
        |
        +---- Users exist ----> Continue
        |
        +---- No users -------> Create Admin
                                      |
                                      v
                              Admin credentials
                                      |
                                      v
                               Store in database
```

The function checks:

```cpp
if (db.hasAnyUsers())
{
    return;
}
```

If no users exist, the system asks for:

* Admin name
* Admin username
* Admin password

and stores the new administrator using:

```cpp
db.addUser(name, username, password, "ADMIN")
```

---

# 12. Loading Market Data

The system then loads stock information:

```cpp
if(!market.loadStocks())
{
    ...
}
```

If loading fails, the system prints a warning but continues running.

The implementation specifically indicates that live data may require an API key configured through `.env`.

This means market-data failure does not necessarily prevent the rest of the TMS from starting.

---

# 13. Initialization Result

When all required initialization steps are completed:

```cpp
return true;
```

The system reports:

```text
Trading System initialized successfully.
```

Therefore:

| Situation                     | Result                    |
| ----------------------------- | ------------------------- |
| Database table creation fails | Initialization fails      |
| No users exist                | First admin is created    |
| Stock loading succeeds        | Continue normally         |
| Stock loading fails           | Warning, system continues |
| Initialization completes      | `true`                    |

---

# 14. Loading Client State

## Function

```cpp
bool TradingSystem::loadClientState(
    int clientId,
    Portfolio& portfolio,
    Collateral& collateral
)
```

This function restores a client's persistent state from the database.

It loads:

* Portfolio holdings
* Available collateral
* Blocked collateral
* Total collateral information

The state is then copied into the in-memory `Portfolio` and `Collateral` objects.

---

# 15. Loading Portfolio

The function first creates:

```cpp
std::vector<std::tuple<std::string, int, double>> holdings;
```

The database loads detailed portfolio information:

```cpp
db.loadPortfolioDetailed(clientId, holdings)
```

Each entry contains:

```text
Symbol
Quantity
Average Price
```

The function then calls:

```cpp
portfolio.setHolding(
    symbol,
    quantity,
    averagePrice
);
```

for every holding.

---

# 16. Loading Collateral

The system loads collateral information using:

```cpp
db.loadCollateral(
    clientId,
    available,
    blocked,
    total
);
```

Three values are retrieved:

```text
available
blocked
total
```

The implementation then restores the in-memory collateral state.

For available collateral:

```cpp
collateral.deposit(available);
```

For blocked collateral:

```cpp
collateral.deposit(blocked);
collateral.reserve(blocked);
```

This reconstructs the distinction between available and blocked collateral.

---

# 17. Client State Restoration

The complete restoration process is:

```text
                 Database
                    |
          +---------+---------+
          |                   |
          v                   v
      Portfolio            Collateral
          |                   |
          v                   v
   Load holdings       Load available
   Load quantity       Load blocked
   Load avg price            |
          |                   |
          v                   v
    setHolding()       deposit/reserve()
          |                   |
          +---------+---------+
                    |
                    v
             Client State
```

This allows the trading engine to work with the client's current state instead of starting from an empty portfolio every time.

---

# 18. Matching Engine

## Function

```cpp
bool TradingSystem::runMatchingEngine()
```

This function is responsible for finding executable orders and passing matched orders to the trade execution module.

The process is:

```text
Database
   |
   v
Load Orders
   |
   v
Filter Pending / Partially Filled
   |
   v
Order Book
   |
   v
MatchMaking
   |
   v
Matched Buy + Sell
   |
   v
Load Buyer/Seller State
   |
   v
Trade Execution
   |
   v
Database Updates
```

---

# 19. Loading Orders

The matching engine first loads orders from the database:

```cpp
std::vector<Order> databaseOrders;

if (!db.loadOrders(databaseOrders))
{
    return false;
}
```

The order book is then cleared:

```cpp
orderBook.clear();
```

---

# 20. Rebuilding the Order Book

Only orders with these statuses are added:

```cpp
OrderStatus::PENDING
```

or:

```cpp
OrderStatus::PARTIALLY_FILLED
```

The implementation checks:

```cpp
if (order.getStatus() == OrderStatus::PENDING
    || order.getStatus() == OrderStatus::PARTIALLY_FILLED)
{
    orderBook.addOrder(order);
}
```

This prevents completed, cancelled, or failed orders from being included in the active matching process.

---

# 21. Obtaining Buy and Sell Orders

After rebuilding the order book:

```cpp
std::vector<Order> buyOrders =
    orderBook.getBuyOrders();

std::vector<Order> sellOrders =
    orderBook.getSellOrders();
```

The matching engine now has two sets of orders:

```text
Buy Orders
    |
    +-- Buy Order 1
    +-- Buy Order 2
    +-- Buy Order 3

Sell Orders
    |
    +-- Sell Order 1
    +-- Sell Order 2
    +-- Sell Order 3
```

---

# 22. Matching Loop

The system repeatedly calls:

```cpp
matchmaking.matchOrders(
    buyOrders,
    sellOrders
);
```

The loop continues until there is no executable match.

```text
                 Start
                   |
                   v
              Match Orders
                   |
          +--------+--------+
          |                 |
       Matched          No Match
          |                 |
          v                 v
   Execute Trade           Stop
          |
          v
    Match Again
```

The implementation uses:

```cpp
while (true)
```

and breaks when:

```cpp
!match.matched
```

or when either matched order pointer is null.

---

# 23. Buyer and Seller State

After a match is found, the system creates:

```cpp
Portfolio buyerPortfolio(...);
Portfolio sellerPortfolio(...);

Collateral buyerCollateral(...);
Collateral sellerCollateral(...);
```

The buyer and seller states are restored from the database using:

```cpp
loadClientState(...)
```

This ensures that trade execution operates on the latest persistent portfolio and collateral information.

---

# 24. Trade Execution

The matched orders are passed to:

```cpp
tradeExecution.executeTrade(
    *match.buyOrder,
    *match.sellOrder,
    buyerPortfolio,
    sellerPortfolio,
    buyerCollateral,
    sellerCollateral
);
```

The execution layer therefore receives:

```text
Buy Order
Sell Order
Buyer Portfolio
Seller Portfolio
Buyer Collateral
Seller Collateral
```

---

# 25. Failed Trade Handling

If trade execution fails:

```cpp
if (!executed)
{
    ...
}
```

both orders are marked:

```cpp
OrderStatus::FAILED
```

The database is then updated:

```cpp
db.updateOrderStatus(...)
```

for both orders.

The matching loop then continues to search for another possible match.

---

# 26. Admin Session

## Function

```cpp
void TradingSystem::runAdminSession()
```

This function controls the interactive administrator menu.

The system first checks:

```cpp
if (!currentUser)
{
    return;
}
```

It then repeatedly displays the administrator menu until logout.

---

# 27. Admin Menu

The implemented admin options are:

```text
+----+-----------------------------+
| No | Operation                   |
+----+-----------------------------+
| 1  | View users                  |
| 2  | Add user                    |
| 3  | Remove user                 |
| 4  | Reset password              |
| 5  | View orders                 |
| 6  | Open market                 |
| 7  | Close market                |
| 8  | Logout                      |
+----+-----------------------------+
```

The menu is displayed through:

```cpp
currentUser->displayMenu();
```

and the selected option is processed using a `switch` statement.

---

# 28. Admin User Management

The administrator can:

### View users

```cpp
printUsers(db);
```

### Add users

The administrator can specify:

```text
Name
Username
Password
Role
```

The role is validated against:

```text
ADMIN
CLIENT
```

The input is converted to uppercase before validation.

### Remove users

```cpp
admin->removeUser(db, userId);
```

### Reset passwords

```cpp
admin->resetPassword(db, userId, newPassword);
```

---

# 29. Market Control

The administrator can open and close the market.

Open:

```cpp
market.setMarketStatus(true);
```

Close:

```cpp
market.setMarketStatus(false);
```

The corresponding menu actions display:

```text
Market opened.
```

or:

```text
Market closed.
```

---

# 30. Client Session

## Function

```cpp
void TradingSystem::runClientSession()
```

This function manages the interactive client menu.

The system first converts the current user to a `Client`:

```cpp
Client* client =
    dynamic_cast<Client*>(currentUser.get());
```

If the conversion fails, the session ends.

---

# 31. Client Menu

The implemented client options are:

```text
+----+-----------------------------+
| No | Operation                   |
+----+-----------------------------+
| 1  | View portfolio              |
| 2  | Place buy order             |
| 3  | Place sell order            |
| 4  | View collateral             |
| 5  | Deposit collateral          |
| 6  | Withdraw collateral         |
| 7  | Modify order                |
| 8  | Cancel order                |
| 9  | Logout                      |
+----+-----------------------------+
```

These operations are handled by calling methods on the `Client` object.

---

# 32. Viewing Portfolio

The client can view their portfolio using:

```cpp
client->viewPortfolio();
```

This delegates portfolio display to the `Client` class.

---

# 33. Placing Buy Orders

The client enters:

```text
Symbol
Quantity
Price
```

The system calls:

```cpp
client->placeBuyOrder(
    symbol,
    quantity,
    price
);
```

If successful:

```text
Buy order placed successfully.
```

Otherwise:

```text
Failed to place buy order.
```

---

# 34. Placing Sell Orders

The client similarly provides:

```text
Symbol
Quantity
Price
```

The system calls:

```cpp
client->placeSellOrder(
    symbol,
    quantity,
    price
);
```

The result is reported to the user.

---

# 35. Collateral Management

The client can view collateral:

```cpp
client->viewCollateral();
```

They can also deposit:

```cpp
client->depositCollateral(amount);
```

or withdraw:

```cpp
client->withdrawCollateral(amount);
```

The TradingSystem delegates these operations to the `Client` module rather than directly modifying the collateral object.

---

# 36. Order Modification

The client can modify an existing order by providing:

```text
Order ID
New Quantity
New Price
```

The system calls:

```cpp
client->modifyOrder(
    orderId,
    newQuantity,
    newPrice
);
```

---

# 37. Order Cancellation

The client can cancel an order using:

```cpp
client->cancelOrder(orderId);
```

The result is reported as either:

```text
Order cancelled successfully.
```

or:

```text
Failed to cancel order.
```

---

# 38. Main System Loop

## Function

```cpp
void TradingSystem::run()
```

This is the primary runtime loop of the TMS.

At the beginning of every iteration, the matching engine is executed:

```cpp
runMatchingEngine();
```

The main menu is then displayed.

```text
+---------------------+
|    Main Menu        |
+---------------------+
| 1. Login            |
| 2. Exit             |
+---------------------+
```

---

# 39. Main Workflow

The overall runtime process is:

```text
             Application Start
                    |
                    v
          initializeSystem()
                    |
                    v
            System Ready
                    |
                    v
              run()
                    |
                    v
          Run Matching Engine
                    |
                    v
              Main Menu
             /          \
            /            \
        Login            Exit
          |                |
          v                v
       login()          shutdown()
          |
          v
    Verify Credentials
          |
     +----+----+
     |         |
   ADMIN     CLIENT
     |         |
     v         v
 Admin      Client
 Session    Session
     |         |
     +----+----+
          |
        Logout
          |
          v
      Main Menu
```

---

# 40. Login Process

## Function

```cpp
void TradingSystem::login()
```

The user provides:

```text
Username
Password
```

The password is read using:

```cpp
consoleInput::readPassword();
```

The database verifies the credentials:

```cpp
db.verifyUser(
    username,
    password,
    role,
    userId
);
```

If authentication fails:

```text
Invalid username or password.
```

is displayed and the function returns.

---

# 41. Creating the User Object

After successful verification, the system checks the role.

For an administrator:

```cpp
currentUser =
    std::make_unique<Admin>(
        userId,
        "Administrator",
        username,
        password
    );
```

For other users:

```cpp
currentUser =
    std::make_unique<Client>(
        userId,
        "Client",
        username,
        password
    );
```

The role therefore determines which session the user receives.

---

# 42. User Login

After creating the appropriate user object:

```cpp
currentUser->login(
    username,
    password
);
```

If the login fails, the object is destroyed:

```cpp
currentUser.reset();
```

If successful:

```text
Login Successful!
```

is displayed.

The system then checks:

```cpp
currentUser->isAdmin()
```

to determine which session should run.

---

# 43. Logout

Both administrator and client sessions use:

```cpp
currentUser->logout();
currentUser.reset();
```

The pointer is reset after logout, removing the current user from memory.

The session loop is then stopped:

```cpp
sessionActive = false;
```

The system returns to the main menu.

---

# 44. Shutdown

## Function

```cpp
void TradingSystem::shutdown()
```

The shutdown sequence is:

```text
Shutdown requested
       |
       v
Reset current user
       |
       v
Close database
       |
       v
System stopped
```

The implementation performs:

```cpp
currentUser.reset();
db.close();
```

and reports successful shutdown.

---

# 45. Complete System Workflow

The complete TMS workflow can be represented as:

```text
                       +-------------------+
                       | Application Start |
                       +---------+---------+
                                 |
                                 v
                       +-------------------+
                       | System Initialize |
                       +---------+---------+
                                 |
                 +---------------+---------------+
                 |               |               |
                 v               v               v
             Database       First Admin       Market Data
             Tables          Bootstrap           Load
                 |               |               |
                 +---------------+---------------+
                                 |
                                 v
                       +-------------------+
                       |   TradingSystem   |
                       |      run()        |
                       +---------+---------+
                                 |
                                 v
                       +-------------------+
                       | Matching Engine   |
                       +---------+---------+
                                 |
                                 v
                       +-------------------+
                       |    Main Menu      |
                       +---------+---------+
                                 |
                          +------+------+
                          |             |
                          v             v
                        Login          Exit
                          |             |
                          v             v
                   Verify User      Shutdown
                          |
                    +-----+-----+
                    |           |
                    v           v
                  Admin       Client
                    |           |
                    v           v
             Admin Session  Client Session
                    |           |
                    +-----+-----+
                          |
                       Logout
                          |
                          v
                     Main Menu
```

---

# 46. Interaction Between Modules

The `TradingSystem` acts as an integration point between several TMS modules.

```text
                        TradingSystem
                              |
        +---------------------+---------------------+
        |          |           |          |         |
        v          v           v          v         v
    Database    Market     OrderBook  MatchMaking  Users
        |                                   |
        |                                   v
        |                              Match Orders
        |                                   |
        |                                   v
        +--------------------------> TradeExecution
                                            |
                                    +-------+-------+
                                    |               |
                                    v               v
                                Portfolio       Collateral
```

This design keeps specialized responsibilities in separate classes while allowing `TradingSystem` to coordinate the complete process.

---

# 47. Error Handling

The module handles several common failure conditions.

### Database initialization failure

```cpp
return false;
```

### Market data failure

A warning is displayed, but the system continues.

### Invalid menu input

The input stream is cleared and invalid characters are discarded.

```cpp
std::cin.clear();
std::cin.ignore(
    std::numeric_limits<std::streamsize>::max(),
    '\n'
);
```

### Authentication failure

The login attempt is rejected.

### Invalid user role

The system falls back to the client object in the current implementation.

### Failed trade execution

Matched orders are marked as failed and their database status is updated.

---

# 48. Input Validation

The main menu and user sessions check whether the entered menu option is a valid integer.

For example:

```cpp
if (!(std::cin >> choice))
{
    std::cin.clear();
    std::cin.ignore(
        std::numeric_limits<std::streamsize>::max(),
        '\n'
    );
}
```

This prevents invalid text input from permanently breaking the input stream.

---

# 49. Object-Oriented Design

The module makes use of several important C++ object-oriented concepts.

## Encapsulation

Each subsystem is represented by its own class:

```text
Database
Market
OrderBook
MatchMaking
TradeExecution
User
Admin
Client
```

---

## Inheritance

`Admin` and `Client` are handled through the base class:

```cpp
std::unique_ptr<User>
```

This allows the trading system to work with different user types through a common interface.

---

## Polymorphism

The system uses:

```cpp
currentUser->displayMenu();
```

and:

```cpp
currentUser->login(...);
```

through the base `User` pointer.

It also uses:

```cpp
dynamic_cast<Admin*>
```

and:

```cpp
dynamic_cast<Client*>
```

when role-specific functionality is required.

---

## RAII

The use of:

```cpp
std::unique_ptr<User>
```

provides automatic lifetime management for the active user object.

---

# 50. Dependencies

The module depends on several TMS-specific components:

```text
Market
TradeExecution
MatchMaking
OrderBook
User
Admin
Client
Database
Portfolio
Collateral
Console Input Utility
```

It also uses standard C++ libraries:

```cpp
#include <memory>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>
#include <cctype>
```

No additional external library is directly required by `TradingSystem` itself beyond the dependencies of the modules it includes.

---

# 51. Public API

| Function             | Return Type | Purpose                                 |
| -------------------- | ----------- | --------------------------------------- |
| `TradingSystem()`    | Constructor | Initializes core system components      |
| `initializeSystem()` | `bool`      | Initializes database, users, and market |
| `login()`            | `void`      | Authenticates a user                    |
| `run()`              | `void`      | Starts the main application loop        |
| `shutdown()`         | `void`      | Closes the system cleanly               |

---

# 52. Private Functions

| Function              | Purpose                                         |
| --------------------- | ----------------------------------------------- |
| `loadClientState()`   | Restores portfolio and collateral from database |
| `runMatchingEngine()` | Finds and executes matching orders              |
| `runAdminSession()`   | Controls administrator operations               |
| `runClientSession()`  | Controls client operations                      |

---

# 53. Design Summary

The `TradingSystem` class functions as the **central controller of the TMS application**.

It does not directly implement every trading operation. Instead, it coordinates specialized modules:

```text
                 TradingSystem
                      |
       +--------------+--------------+
       |              |              |
       v              v              v
    Database        Market       User System
       |                             |
       |                    +--------+--------+
       |                    |                 |
       v                    v                 v
   Persistence           Admin             Client
                                         |
                                         v
                                  Trading Operations
                                         |
                         +---------------+---------------+
                         |               |               |
                         v               v               v
                     OrderBook       MatchMaking    Collateral
                         |               |               |
                         +---------------+---------------+
                                         |
                                         v
                                  TradeExecution
                                         |
                                         v
                                     Database
```

The overall responsibility of the module can therefore be summarized as:

```text
Initialize
    ↓
Authenticate
    ↓
Select User Role
    ↓
Run User Session
    ↓
Maintain Order Book
    ↓
Match Orders
    ↓
Execute Trades
    ↓
Persist State
    ↓
Logout / Continue
    ↓
Shutdown
```

---

# 54. Conclusion

The **TradingSystem Module** is the central integration layer of the Trading Management System.

It connects the database, market, order book, matchmaking engine, trade execution engine, portfolio, collateral, and user-management components.

Its most important responsibility is coordinating the complete lifecycle of the application:

```text
System Initialization
        ↓
User Authentication
        ↓
Role-Based Session
        ↓
Order Management
        ↓
Order Matching
        ↓
Trade Execution
        ↓
Portfolio/Collateral State
        ↓
Database Persistence
        ↓
Shutdown
```

By keeping the specialized logic inside dedicated modules and using `TradingSystem` as the coordinator, the project maintains a modular architecture where individual components can be developed and tested independently.
