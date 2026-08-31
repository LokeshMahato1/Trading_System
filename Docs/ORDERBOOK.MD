# Order Book Module

## 1. Overview

The **Order Book Module** is responsible for storing, organizing, searching, modifying, and removing buy and sell orders in the Trading Management System (TMS).

The `OrderBook` maintains two separate collections:

* Buy orders
* Sell orders

Orders are automatically sorted according to **price priority** and **time priority**.

The main class is:

```cpp
class OrderBook
```

---

# 2. Purpose

The primary purpose of the OrderBook is to maintain an organized collection of active trading orders so that the MatchMaking module can efficiently access buy and sell orders.

The basic workflow is:

```text
              New Order
                  |
                  v
            OrderBook
                  |
        +---------+---------+
        |                   |
      BUY                 SELL
        |                   |
        v                   v
   Buy Orders          Sell Orders
        |                   |
        v                   v
  Sort by Price        Sort by Price
        |                   |
        +---------+---------+
                  |
                  v
             MatchMaking
```

---

# 3. File Structure

The OrderBook module consists of:

```text
orderbook/
├── orderBook.hpp
└── orderBook.cpp
```

The header file defines the class interface, while the `.cpp` file implements the functionality.

---

# 4. Dependencies

The header includes:

```cpp
#include <vector>
#include "../order/order.h"
```

The implementation includes:

```cpp
#include "orderBook.hpp"
#include <algorithm>
```

### Dependency purposes

| Dependency    | Purpose                                     |
| ------------- | ------------------------------------------- |
| `<vector>`    | Stores buy and sell orders                  |
| `order.h`     | Provides the `Order` class and `OrderType`  |
| `<algorithm>` | Provides `std::sort()` and `std::find_if()` |

---

# 5. Class Structure

The main class is:

```cpp
class OrderBook
{
private:
    std::vector<Order> buyOrders;
    std::vector<Order> sellOrders;

    void sortBuyOrders();
    void sortSellOrders();

public:
    OrderBook();

    bool addOrder(const Order& order);
    bool cancelOrder(int orderId);
    bool modifyOrder(
        int orderId,
        int newQuantity,
        double newPrice
    );

    Order* findOrder(int orderId);
    const Order* findOrder(int orderId) const;

    const std::vector<Order> getBuyOrders() const;
    const std::vector<Order> getSellOrders() const;

    size_t buySize() const;
    size_t sellSize() const;

    bool empty() const;

    void clear();
};
```

---

# 6. Internal Data Structures

The OrderBook maintains two vectors:

```cpp
std::vector<Order> buyOrders;
std::vector<Order> sellOrders;
```

The separation is based on order type.

```text
OrderBook
   |
   +-------------------+
   |                   |
   v                   v
buyOrders          sellOrders
   |                   |
   v                   v
BUY orders          SELL orders
```

This makes it easier for the MatchMaking module to access the two sides of the market separately.

---

# 7. Buy Order Collection

The `buyOrders` vector stores orders where:

```cpp
order.getType() == OrderType::BUY
```

Buy orders are sorted using:

1. Higher price first
2. Earlier creation time first when prices are equal

This follows a **price-time priority** system.

---

# 8. Sell Order Collection

The `sellOrders` vector stores orders where:

```cpp
order.getType() == OrderType::SELL
```

Sell orders are sorted using:

1. Lower price first
2. Earlier creation time first when prices are equal

This also follows price-time priority.

---

# 9. Constructor

The constructor is:

```cpp
OrderBook::OrderBook() {}
```

It creates an empty order book.

Initially:

```text
Buy Orders  = empty
Sell Orders = empty
```

---

# 10. Buy Order Sorting

The private function:

```cpp
void OrderBook::sortBuyOrders()
```

uses:

```cpp
std::sort()
```

to organize buy orders.

The comparison rule is:

```cpp
if (a.getPrice() == b.getPrice())
{
    return a.getCreatedAt() < b.getCreatedAt();
}

return a.getPrice() > b.getPrice();
```

---

# 11. Buy Order Priority

Buy orders are sorted by **descending price**.

For example:

```text
BUY $150
BUY $140
BUY $130
BUY $120
```

The highest-priced buyer gets priority.

The rule is:

```text
Higher Buy Price = Higher Priority
```

---

# 12. Buy Order Time Priority

If two buy orders have the same price, creation time is used:

```cpp
a.getCreatedAt() < b.getCreatedAt()
```

Therefore, the earlier order gets priority.

Example:

```text
Order A
Price = $150
Created = 10:00

Order B
Price = $150
Created = 10:05
```

The order book becomes:

```text
Order A
Order B
```

because Order A was created first.

This is the **First-In-First-Out (FIFO)** rule for equal prices.

---

# 13. Sell Order Sorting

The private function:

```cpp
void OrderBook::sortSellOrders()
```

also uses `std::sort()`.

The comparison rule is:

```cpp
if (a.getPrice() == b.getPrice())
{
    return a.getCreatedAt() < b.getCreatedAt();
}

return a.getPrice() < b.getPrice();
```

---

# 14. Sell Order Priority

Sell orders are sorted by **ascending price**.

For example:

```text
SELL $100
SELL $105
SELL $110
SELL $120
```

The lowest-priced seller gets priority.

The rule is:

```text
Lower Sell Price = Higher Priority
```

---

# 15. Sell Order Time Priority

If two sell orders have the same price, creation time determines their order.

Example:

```text
Order A
Price = $100
Created = 10:00

Order B
Price = $100
Created = 10:10
```

The order book places:

```text
Order A
Order B
```

because Order A was created first.

---

# 16. Price-Time Priority

The OrderBook therefore follows:

```text
                 OrderBook Priority
                        |
              +---------+---------+
              |                   |
              v                   v
           BUY SIDE           SELL SIDE
              |                   |
              v                   v
        Highest Price         Lowest Price
              |                   |
              v                   v
       Earlier Time          Earlier Time
```

This can be summarized as:

| Side | Primary Priority | Secondary Priority     |
| ---- | ---------------- | ---------------------- |
| BUY  | Highest price    | Earliest creation time |
| SELL | Lowest price     | Earliest creation time |

---

# 17. `addOrder()`

The function:

```cpp
bool OrderBook::addOrder(const Order& order)
```

adds a new order to the appropriate side of the order book.

The order type determines where it is stored.

---

# 18. Adding a Buy Order

If:

```cpp
order.getType() == OrderType::BUY
```

the order is added to:

```cpp
buyOrders.push_back(order);
```

The buy orders are then sorted:

```cpp
sortBuyOrders();
```

The function returns:

```cpp
return true;
```

Workflow:

```text
BUY Order
    |
    v
buyOrders.push_back()
    |
    v
sortBuyOrders()
    |
    v
Buy Order Book Updated
```

---

# 19. Adding a Sell Order

If:

```cpp
order.getType() == OrderType::SELL
```

the order is added to:

```cpp
sellOrders.push_back(order);
```

Then:

```cpp
sortSellOrders();
```

is called.

Workflow:

```text
SELL Order
    |
    v
sellOrders.push_back()
    |
    v
sortSellOrders()
    |
    v
Sell Order Book Updated
```

---

# 20. Invalid Order Type

If the order is neither BUY nor SELL:

```cpp
else
{
    return false;
}
```

The order is rejected.

Therefore:

```text
Valid:
BUY
SELL

Invalid:
Any other order type
```

---

# 21. `findOrder()`

The OrderBook provides two versions of `findOrder()`:

```cpp
Order* findOrder(int orderId);
```

and:

```cpp
const Order* findOrder(int orderId) const;
```

The purpose is to locate an order using its unique order ID.

---

# 22. Searching Buy Orders

The non-const implementation first searches the buy order vector:

```cpp
for (auto& order : buyOrders)
{
    if (order.getOrderId() == orderId)
    {
        return &order;
    }
}
```

If the order is found, a pointer to the actual order stored in the vector is returned.

---

# 23. Searching Sell Orders

If the order is not found in the buy orders, the function searches the sell orders:

```cpp
for (auto& order : sellOrders)
{
    if (order.getOrderId() == orderId)
    {
        return &order;
    }
}
```

If found:

```cpp
return &order;
```

---

# 24. Order Not Found

If the order does not exist in either collection:

```cpp
return nullptr;
```

Therefore:

```text
Order Found
    |
    v
Return pointer

Order Not Found
    |
    v
Return nullptr
```

---

# 25. Why `findOrder()` Returns a Pointer

The function returns:

```cpp
Order*
```

rather than an `Order` object.

This allows the caller to modify the actual order stored inside the OrderBook.

For example:

```cpp
Order* order = findOrder(orderId);

if (order != nullptr)
{
    order->setPrice(newPrice);
}
```

The original order inside the vector is modified.

---

# 26. Const Version of `findOrder()`

The class also provides:

```cpp
const Order* findOrder(int orderId) const;
```

The const version can be used when the `OrderBook` itself is const.

It prevents the caller from modifying the returned order through the pointer.

Conceptually:

```text
Non-const OrderBook
       |
       v
Order*
       |
       v
Can modify order


Const OrderBook
       |
       v
const Order*
       |
       v
Read only
```

---

# 27. `cancelOrder()`

The function:

```cpp
bool OrderBook::cancelOrder(int orderId)
```

removes an order from the order book using its ID.

It first searches the buy orders.

---

# 28. Canceling a Buy Order

The function uses `std::find_if()`:

```cpp
auto buyIterator =
    std::find_if(
        buyOrders.begin(),
        buyOrders.end(),
        [orderId](const Order& order) {
            return order.getOrderId() == orderId;
        }
    );
```

If the order is found:

```cpp
buyOrders.erase(buyIterator);
return true;
```

The order is permanently removed from the in-memory OrderBook.

---

# 29. Canceling a Sell Order

If the order was not found in the buy collection, the sell collection is searched.

```cpp
auto sellIterator =
    std::find_if(
        sellOrders.begin(),
        sellOrders.end(),
        [orderId](const Order& order) {
            return order.getOrderId() == orderId;
        }
    );
```

If found:

```cpp
sellOrders.erase(sellIterator);
return true;
```

---

# 30. Cancel Order Failure

If the order does not exist in either collection:

```cpp
return false;
```

The result is:

```text
Order Exists
    |
    v
Remove Order
    |
    v
true

Order Does Not Exist
    |
    v
false
```

---

# 31. `modifyOrder()`

The function:

```cpp
bool OrderBook::modifyOrder(
    int orderId,
    int newQuantity,
    double newPrice
)
```

changes:

* Remaining quantity
* Price

of an existing order.

---

# 32. Finding the Order to Modify

The function first calls:

```cpp
Order* order = findOrder(orderId);
```

If no order is found:

```cpp
if (order == nullptr)
{
    return false;
}
```

---

# 33. Updating Quantity

If the order exists:

```cpp
order->setRemainingQuantity(newQuantity);
```

The order's remaining quantity is replaced with the new value.

---

# 34. Updating Price

The new price is assigned using:

```cpp
order->setPrice(newPrice);
```

The order therefore contains the updated price.

---

# 35. Re-Sorting After Modification

Because changing an order's price can change its priority, the order book is sorted again.

For a buy order:

```cpp
if (order->getType() == OrderType::BUY)
{
    sortBuyOrders();
}
```

For a sell order:

```cpp
else
{
    sortSellOrders();
}
```

Therefore:

```text
Modify Order
     |
     +---- Change Quantity
     |
     +---- Change Price
     |
     v
Re-sort OrderBook
     |
     v
Updated Priority
```

---

# 36. `getBuyOrders()`

The function:

```cpp
const std::vector<Order>
OrderBook::getBuyOrders() const
```

returns the buy order collection.

Implementation:

```cpp
return buyOrders;
```

The returned vector is a copy of the internal vector.

---

# 37. `getSellOrders()`

Similarly:

```cpp
const std::vector<Order>
OrderBook::getSellOrders() const
```

returns the sell order collection.

Implementation:

```cpp
return sellOrders;
```

It also returns a copy.

---

# 38. Important Behavior of the Accessors

The functions are declared:

```cpp
const std::vector<Order> getBuyOrders() const;
const std::vector<Order> getSellOrders() const;
```

Therefore, callers receive copies rather than direct references to the internal vectors.

Conceptually:

```text
Internal Buy Orders
        |
        v
getBuyOrders()
        |
        v
Copied Buy Orders
```

and:

```text
Internal Sell Orders
        |
        v
getSellOrders()
        |
        v
Copied Sell Orders
```

Changes made to the returned vector do not directly modify the internal OrderBook.

---

# 39. `buySize()`

The function:

```cpp
size_t OrderBook::buySize() const
```

returns the number of buy orders.

Implementation:

```cpp
return buyOrders.size();
```

Example:

```text
Buy Orders:
AAPL
MSFT
TSLA

buySize() = 3
```

---

# 40. `sellSize()`

The function:

```cpp
size_t OrderBook::sellSize() const
```

returns the number of sell orders.

Implementation:

```cpp
return sellOrders.size();
```

Example:

```text
Sell Orders:
AAPL
NVDA

sellSize() = 2
```

---

# 41. `empty()`

The function:

```cpp
bool OrderBook::empty() const
```

checks whether both order collections are empty.

Implementation:

```cpp
return buyOrders.empty() && sellOrders.empty();
```

Therefore:

```text
Buy Orders = Empty
AND
Sell Orders = Empty
```

must both be true.

---

# 42. Empty OrderBook Example

```text
Buy Orders  = 0
Sell Orders = 0
```

Then:

```cpp
orderBook.empty()
```

returns:

```text
true
```

If either side contains an order:

```text
Buy Orders  = 1
Sell Orders = 0
```

then:

```cpp
orderBook.empty()
```

returns:

```text
false
```

---

# 43. `clear()`

The function:

```cpp
void OrderBook::clear()
```

removes all orders from both sides.

Implementation:

```cpp
buyOrders.clear();
sellOrders.clear();
```

Workflow:

```text
OrderBook
   |
   +---- Buy Orders
   |
   +---- Sell Orders
   |
   v
clear()
   |
   +---- Remove all Buy Orders
   |
   +---- Remove all Sell Orders
```

After `clear()`:

```text
buyOrders  = empty
sellOrders = empty
```

---

# 44. Complete OrderBook Workflow

The complete order lifecycle inside the OrderBook is:

```text
                    New Order
                        |
                        v
                   addOrder()
                        |
              +---------+---------+
              |                   |
             BUY                SELL
              |                   |
              v                   v
         buyOrders           sellOrders
              |                   |
              v                   v
       sortBuyOrders()     sortSellOrders()
              |                   |
              +---------+---------+
                        |
                        v
                   OrderBook
                        |
          +-------------+-------------+
          |             |             |
          v             v             v
      findOrder()  modifyOrder()  cancelOrder()
          |             |             |
          |             v             |
          |          Re-sort          |
          |                           |
          +-------------+-------------+
                        |
                        v
                    MatchMaking
```

---

# 45. Order Priority Example

Consider the following buy orders:

```text
Order A: $100, 10:05
Order B: $105, 10:10
Order C: $105, 10:02
Order D: $95,  10:01
```

After sorting:

```text
1. Order C → $105, 10:02
2. Order B → $105, 10:10
3. Order A → $100, 10:05
4. Order D → $95,  10:01
```

The reason is:

```text
Highest Price First
       +
Earlier Time First
```

for equal prices.

---

# 46. Sell Order Priority Example

Consider:

```text
Order A: $100, 10:05
Order B: $95,  10:10
Order C: $95,  10:02
Order D: $105, 10:01
```

After sorting:

```text
1. Order C → $95,  10:02
2. Order B → $95,  10:10
3. Order A → $100, 10:05
4. Order D → $105, 10:01
```

The reason is:

```text
Lowest Price First
       +
Earlier Time First
```

for equal prices.

---

# 47. Interaction With MatchMaking

The OrderBook supplies the orders that are evaluated by MatchMaking.

The relationship is:

```text
                 OrderBook
                /         \
               v           v
         Buy Orders      Sell Orders
               \           /
                \         /
                 v       v
                MatchMaking
                     |
                     v
                 MatchResult
```

The OrderBook is responsible for **maintaining order priority**, while MatchMaking is responsible for **determining whether two orders can match**.

---

# 48. Interaction With TradeExecution

After MatchMaking finds a valid pair:

```text
OrderBook
    |
    v
MatchMaking
    |
    v
MatchResult
    |
    v
TradeExecution
```

TradeExecution then updates the order quantities and statuses.

The OrderBook's stored orders can subsequently reflect those changes when the appropriate order objects are updated.

---

# 49. Responsibilities of OrderBook

The OrderBook handles:

```text
✓ Store buy orders
✓ Store sell orders
✓ Sort buy orders
✓ Sort sell orders
✓ Add orders
✓ Find orders
✓ Cancel orders
✓ Modify orders
✓ Count buy orders
✓ Count sell orders
✓ Check whether book is empty
✓ Clear the order book
```

---

# 50. Responsibilities Not Handled by OrderBook

The OrderBook does not perform:

```text
✗ Order matching
✗ Trade execution
✗ Portfolio updates
✗ Collateral management
✗ Transaction recording
✗ Database persistence
```

Those responsibilities belong to other modules.

---

# 51. Module Responsibility Separation

The TMS separates responsibilities between modules:

```text
OrderBook
    |
    +-- Stores and prioritizes orders
              |
              v
MatchMaking
    |
    +-- Determines compatible orders
              |
              v
TradeExecution
    |
    +-- Executes matched trade
              |
              v
Portfolio + Collateral
    |
    +-- Updates client state
              |
              v
Database
    |
    +-- Persists system state
```

This separation makes each component responsible for a specific part of the trading workflow.

---

# 52. Function Reference

| Function           | Visibility | Purpose                             |
| ------------------ | ---------- | ----------------------------------- |
| `OrderBook()`      | Public     | Creates an empty order book         |
| `sortBuyOrders()`  | Private    | Sorts buy orders by price and time  |
| `sortSellOrders()` | Private    | Sorts sell orders by price and time |
| `addOrder()`       | Public     | Adds a buy or sell order            |
| `cancelOrder()`    | Public     | Removes an order by ID              |
| `modifyOrder()`    | Public     | Changes quantity and price          |
| `findOrder()`      | Public     | Finds an order by ID                |
| `getBuyOrders()`   | Public     | Returns buy order collection        |
| `getSellOrders()`  | Public     | Returns sell order collection       |
| `buySize()`        | Public     | Returns number of buy orders        |
| `sellSize()`       | Public     | Returns number of sell orders       |
| `empty()`          | Public     | Checks whether the book is empty    |
| `clear()`          | Public     | Removes all orders                  |

---

# 53. OrderBook State Model

The OrderBook can be represented as:

```text
                       ORDER BOOK
                           |
              +------------+------------+
              |                         |
              v                         v
          BUY SIDE                   SELL SIDE
              |                         |
              v                         v
       Highest Price               Lowest Price
              |                         |
              v                         v
       Earlier Time               Earlier Time
              |                         |
              +------------+------------+
                           |
                           v
                       MatchMaking
```

---

# 54. Example Complete Scenario

Suppose the following orders are submitted:

```text
BUY:
User 101
AAPL
Quantity = 100
Price = $105

SELL:
User 202
AAPL
Quantity = 60
Price = $100
```

### Step 1 — Add Buy Order

```text
addOrder(BUY)
        |
        v
buyOrders
        |
        v
sortBuyOrders()
```

### Step 2 — Add Sell Order

```text
addOrder(SELL)
        |
        v
sellOrders
        |
        v
sortSellOrders()
```

### Step 3 — MatchMaking

The MatchMaking module checks:

```text
Same Symbol       → YES
Different Users   → YES
105 >= 100        → YES
Active Orders     → YES
Quantity > 0      → YES
```

The orders can match.

### Step 4 — Matched Quantity

```text
min(100, 60) = 60
```

### Step 5 — Trade Execution

The TradeExecution module executes:

```text
60 AAPL shares
at $100
```

The OrderBook therefore forms the order-maintenance layer before matching and execution.

---

# 55. Overall TMS Architecture

The OrderBook fits into the broader Trading Management System architecture as follows:

```text
                         Trading System
                               |
                               v
                           OrderBook
                               |
                 +-------------+-------------+
                 |                           |
                 v                           v
             Buy Orders                 Sell Orders
                 |                           |
                 +-------------+-------------+
                               |
                               v
                         MatchMaking
                               |
                               v
                         MatchResult
                               |
                               v
                        TradeExecution
                               |
              +----------------+----------------+
              |                |                |
              v                v                v
          Portfolio        Collateral        Orders
              |                |                |
              +----------------+----------------+
                               |
                               v
                           Database
                               |
                               v
                       Transaction History
```

---

# 56. Summary

The **OrderBook Module** provides the central in-memory structure for managing buy and sell orders.

Its key features are:

```text
                OrderBook
                    |
        +-----------+-----------+
        |                       |
        v                       v
    BUY ORDERS              SELL ORDERS
        |                       |
        v                       v
 High Price First         Low Price First
        |                       |
        v                       v
 Earlier Time First       Earlier Time First
        |                       |
        +-----------+-----------+
                    |
                    v
               MatchMaking
```

The module supports the complete basic order-management lifecycle:

```text
Add
 ↓
Sort
 ↓
Find
 ↓
Modify
 ↓
Re-sort
 ↓
Cancel
 ↓
Clear
```

In the overall TMS architecture:

```text
OrderBook
    ↓
MatchMaking
    ↓
TradeExecution
    ↓
Portfolio / Collateral
    ↓
Database
```

The OrderBook therefore acts as the **order-management and price-time-priority layer** of the Trading Management System.
