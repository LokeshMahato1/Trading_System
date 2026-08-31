# MatchMaking Module

## 1. Overview

The **MatchMaking Module** is responsible for finding compatible buy and sell orders in the Trading Management System (TMS).

It checks whether two orders can be matched based on:

* Stock symbol
* Client/user identity
* Buy and sell prices
* Order status
* Remaining quantity

When a valid pair is found, the module returns the matched buy order, sell order, matched quantity, and execution price.

The main class is:

```cpp
class MatchMaking
```

---

# 2. Purpose

The primary purpose of the `MatchMaking` module is to determine whether an active buy order and an active sell order can be executed against each other.

The basic workflow is:

```text
Buy Orders
    |
    |
    v
MatchMaking
    ^
    |
    |
Sell Orders
    |
    v
Check Matching Conditions
    |
    +---- Symbol
    +---- Different Users
    +---- Price
    +---- Status
    +---- Quantity
    |
    v
Valid Match
    |
    +---- Buy Order
    +---- Sell Order
    +---- Matched Quantity
    +---- Execution Price
```

---

# 3. File Structure

The MatchMaking module consists of:

```text
Matchmaking/
├── matchmaking.h
└── matchmaking.cpp
```

The header file contains the class and function declarations, while the `.cpp` file contains the implementation.

---

# 4. Dependencies

The implementation includes:

```cpp
#include "matchmaking.h"
#include <algorithm>
```

### Dependency purposes

| Dependency      | Purpose                                                     |
| --------------- | ----------------------------------------------------------- |
| `matchmaking.h` | Defines the `MatchMaking` class and `MatchResult` structure |
| `<algorithm>`   | Provides `std::min()` for calculating matched quantity      |

The module also depends indirectly on the `Order` class and `OrderStatus` enumeration through `matchmaking.h`.

---

# 5. Constructor

The constructor is:

```cpp
MatchMaking::MatchMaking() {}
```

It does not require any external objects or configuration.

Example:

```cpp
MatchMaking matchmaking;
```

The object can then be used to compare and match buy and sell orders.

---

# 6. Main Responsibilities

The MatchMaking module has three main operations:

```text
isMatch()
    |
    +-- Determines whether two orders are compatible

calculateMatchedQuantity()
    |
    +-- Determines how many shares can be matched

determineExecutionPrice()
    |
    +-- Determines the price at which the trade will execute

matchOrders()
    |
    +-- Searches buy and sell order collections
        and returns the first valid match
```

---

# 7. `isMatch()`

The function:

```cpp
bool MatchMaking::isMatch(
    const Order& buyOrder,
    const Order& sellOrder
)
```

determines whether a particular buy order and sell order can be matched.

It performs several validation checks.

The function returns:

```text
true
```

if all matching conditions are satisfied.

Otherwise it returns:

```text
false
```

---

# 8. Matching Condition 1: Stock Symbol

The first check compares the stock symbols:

```cpp
if (buyOrder.getStockSymbol() != sellOrder.getStockSymbol())
{
    return false;
}
```

Both orders must belong to the same stock.

### Valid

```text
BUY  AAPL
SELL AAPL
```

### Invalid

```text
BUY  AAPL
SELL MSFT
```

Therefore:

```text
Buy Symbol == Sell Symbol
```

is required.

---

# 9. Matching Condition 2: Prevent Self-Matching

The module prevents a client from matching their own orders:

```cpp
if (buyOrder.getUserId() == sellOrder.getUserId())
{
    return false;
}
```

This means the buyer and seller must have different user IDs.

### Valid

```text
Buyer User ID  = 101
Seller User ID = 202
```

### Invalid

```text
Buyer User ID  = 101
Seller User ID = 101
```

This prevents self-trading.

---

# 10. Matching Condition 3: Price

The buy order price must be greater than or equal to the sell order price:

```cpp
if (buyOrder.getPrice() < sellOrder.getPrice())
{
    return false;
}
```

The matching condition is therefore:

```text
Buy Price >= Sell Price
```

### Example 1

```text
Buy Price  = $105
Sell Price = $100

105 >= 100
```

The orders can match.

### Example 2

```text
Buy Price  = $95
Sell Price = $100

95 < 100
```

The orders cannot match.

---

# 11. Matching Condition 4: Order Status

Both orders must be active.

The module rejects a buy order if its status is:

* `CANCELLED`
* `FILLED`
* `FAILED`

The check is:

```cpp
if (
    buyOrder.getStatus() == OrderStatus::CANCELLED ||
    buyOrder.getStatus() == OrderStatus::FILLED ||
    buyOrder.getStatus() == OrderStatus::FAILED
)
{
    return false;
}
```

The same validation is applied to the sell order:

```cpp
if (
    sellOrder.getStatus() == OrderStatus::CANCELLED ||
    sellOrder.getStatus() == OrderStatus::FILLED ||
    sellOrder.getStatus() == OrderStatus::FAILED
)
{
    return false;
}
```

---

# 12. Active Order Status

Orders with the following statuses are rejected:

```text
CANCELLED
FILLED
FAILED
```

Therefore, an order with:

```text
PENDING
```

can participate in matching.

An order with:

```text
PARTIALLY_FILLED
```

can also participate because it may still have a remaining quantity.

The important requirement is that the order must not be cancelled, fully filled, or failed.

---

# 13. Matching Condition 5: Remaining Quantity

The module checks that both orders still have shares available:

```cpp
if (buyOrder.getRemainingQuantity() <= 0)
{
    return false;
}

if (sellOrder.getRemainingQuantity() <= 0)
{
    return false;
}
```

Both conditions must satisfy:

```text
Buy Remaining Quantity > 0
Sell Remaining Quantity > 0
```

An order with zero remaining quantity cannot participate in another trade.

---

# 14. Successful Match

If all checks pass:

```cpp
return true;
```

The orders are considered compatible.

The complete matching condition can therefore be represented as:

```text
Same Symbol
     AND
Different Users
     AND
Buy Price >= Sell Price
     AND
Buy Order Active
     AND
Sell Order Active
     AND
Buy Remaining Quantity > 0
     AND
Sell Remaining Quantity > 0
```

---

# 15. `calculateMatchedQuantity()`

The function is:

```cpp
int MatchMaking::calculateMatchedQuantity(
    const Order& buyOrder,
    const Order& sellOrder
) const
```

It determines how many shares can actually be exchanged.

The implementation is:

```cpp
return std::min(
    buyOrder.getRemainingQuantity(),
    sellOrder.getRemainingQuantity()
);
```

Therefore:

```text
Matched Quantity
=
Minimum(Buy Remaining, Sell Remaining)
```

---

# 16. Matched Quantity Example

Suppose:

```text
Buy Remaining  = 100
Sell Remaining = 70
```

Then:

```text
Matched Quantity
=
min(100, 70)
=
70
```

After execution:

```text
Buy Remaining  = 30
Sell Remaining = 0
```

The sell order becomes completely filled while the buy order can remain partially filled.

---

# 17. Another Matched Quantity Example

Suppose:

```text
Buy Remaining  = 50
Sell Remaining = 100
```

Then:

```text
Matched Quantity = 50
```

After execution:

```text
Buy Remaining  = 0
Sell Remaining = 50
```

The buy order becomes filled and the sell order remains partially filled.

---

# 18. `determineExecutionPrice()`

The function is:

```cpp
double MatchMaking::determineExecutionPrice(
    const Order& /*buyOrder*/,
    const Order& sellOrder
) const
```

The implementation returns:

```cpp
return sellOrder.getPrice();
```

Therefore:

```text
Execution Price = Sell Order Price
```

The buy order parameter is intentionally unused.

---

# 19. Execution Price Example

Suppose:

```text
Buy Order Price  = $105
Sell Order Price = $100
```

Since:

```text
105 >= 100
```

the orders can match.

The execution price is:

```text
$100
```

not `$105`.

Therefore, the buyer receives the stock at the seller's resting order price.

---

# 20. Why the Buy Order Parameter Is Commented Out

The function contains:

```cpp
const Order& /*buyOrder*/
```

The parameter is still part of the function interface, but its name is commented out to indicate that it is intentionally unused.

This avoids an unused-parameter warning while keeping the function interface consistent.

---

# 21. `matchOrders()`

The main search function is:

```cpp
MatchResult MatchMaking::matchOrders(
    std::vector<Order>& buyOrders,
    std::vector<Order>& sellOrders
)
```

It searches the buy and sell order collections for a valid match.

The function creates:

```cpp
MatchResult result;
```

The result initially represents no match.

---

# 22. Searching Buy and Sell Orders

The function uses nested loops:

```cpp
for (Order& buy : buyOrders)
{
    for (Order& sell : sellOrders)
    {
        ...
    }
}
```

This means the system checks each buy order against each sell order.

Conceptually:

```text
Buy Orders
   |
   +-- Buy 1 ----+
   |             |
   +-- Buy 2 ----+---- Compare with Sell Orders
   |             |
   +-- Buy 3 ----+
```

For each buy order:

```text
Buy 1 → Sell 1
Buy 1 → Sell 2
Buy 1 → Sell 3
...
Buy 2 → Sell 1
Buy 2 → Sell 2
...
```

until a valid match is found.

---

# 23. Checking Each Pair

For every buy/sell combination, the function calls:

```cpp
if (!isMatch(buy, sell))
{
    continue;
}
```

If the orders do not match, the `continue` statement skips that sell order and checks the next one.

```text
Buy + Sell
    |
    v
 isMatch()
    |
    +---- false → Check next pair
    |
    +---- true
            |
            v
        Match Found
```

---

# 24. Creating the Match Result

When a valid pair is found:

```cpp
result.matched = true;
```

This indicates that a valid order match exists.

---

# 25. Storing the Buy Order

The result stores a pointer to the matched buy order:

```cpp
result.buyOrder = &buy;
```

The pointer refers to the actual order inside the `buyOrders` vector.

This allows the execution stage to work with the original order object.

---

# 26. Storing the Sell Order

Similarly:

```cpp
result.sellOrder = &sell;
```

stores a pointer to the matched sell order.

The result therefore contains references to both orders that should be executed.

---

# 27. Storing Matched Quantity

The matched quantity is calculated:

```cpp
result.matchedQuantity =
    calculateMatchedQuantity(buy, sell);
```

For example:

```text
Buy remaining  = 100
Sell remaining = 60

Matched quantity = 60
```

---

# 28. Storing Execution Price

The execution price is calculated:

```cpp
result.executionPrice =
    determineExecutionPrice(buy, sell);
```

Since the current implementation uses the seller's price:

```text
Execution Price = Sell Price
```

---

# 29. Returning the Match

Immediately after finding a valid match:

```cpp
return result;
```

This means the function stops searching after the **first valid match**.

It does not continue searching for additional matches during the same function call.

---

# 30. No Match Found

If the nested loops finish without finding a valid pair:

```cpp
return result;
```

The returned `MatchResult` remains in its default state.

This indicates that no compatible orders were found.

---

# 31. Match Result Workflow

The returned result contains:

```text
MatchResult
    |
    +-- matched
    |
    +-- buyOrder
    |
    +-- sellOrder
    |
    +-- matchedQuantity
    |
    +-- executionPrice
```

Conceptually:

```text
Buy Orders + Sell Orders
          |
          v
      matchOrders()
          |
          v
       isMatch()
          |
     +----+----+
     |         |
   False      True
     |         |
     v         v
 Continue    Create
 Search      MatchResult
               |
       +-------+-------+
       |       |       |
       v       v       v
      Buy    Sell   Quantity
     Order   Order
               |
               v
        Execution Price
```

---

# 32. Relationship With OrderBook

The MatchMaking module works with orders supplied by the order-book system.

Conceptually:

```text
                 OrderBook
                /         \
               /           \
        Buy Orders       Sell Orders
               \           /
                \         /
                 v       v
                MatchMaking
                     |
                     v
                MatchResult
```

The OrderBook maintains orders, while MatchMaking determines whether orders can be matched.

---

# 33. Relationship With TradeExecution

MatchMaking does not perform the actual transfer of money or shares.

Its job is to identify a valid match.

The next stage is `TradeExecution`.

```text
OrderBook
    |
    v
MatchMaking
    |
    | Valid match
    v
MatchResult
    |
    v
TradeExecution
    |
    +---- Update Buyer Portfolio
    |
    +---- Update Seller Portfolio
    |
    +---- Update Buyer Collateral
    |
    +---- Update Seller Collateral
    |
    +---- Update Order Status
    |
    +---- Save Transaction
```

Therefore:

```text
MatchMaking = Find the trade
TradeExecution = Perform the trade
```

---

# 34. Complete Matching Workflow

The complete process is:

```text
                  Buy Orders
                      |
                      |
                      v
              +---------------+
              |  MatchMaking  |
              +-------+-------+
                      ^
                      |
                      |
                  Sell Orders
                      |
                      v
                Check Symbol
                      |
                      v
             Check User IDs
                      |
                      v
              Check Prices
                      |
                      v
              Check Status
                      |
                      v
             Check Quantity
                      |
              +-------+-------+
              |               |
             Fail            Pass
              |               |
              v               v
        No Match         Calculate Quantity
                              |
                              v
                     Determine Price
                              |
                              v
                       MatchResult
                              |
                              v
                       TradeExecution
```

---

# 35. Example: Successful Match

Consider:

```text
Buy Order:
    User ID       = 101
    Symbol        = AAPL
    Price         = $105
    Remaining     = 100
    Status        = PENDING

Sell Order:
    User ID       = 202
    Symbol        = AAPL
    Price         = $100
    Remaining     = 60
    Status        = PENDING
```

### Step 1: Symbol

```text
AAPL == AAPL
```

Pass.

### Step 2: User

```text
101 != 202
```

Pass.

### Step 3: Price

```text
105 >= 100
```

Pass.

### Step 4: Status

Both are `PENDING`.

Pass.

### Step 5: Quantity

```text
100 > 0
60 > 0
```

Pass.

### Step 6: Matched Quantity

```text
min(100, 60) = 60
```

### Step 7: Execution Price

```text
Sell Price = $100
```

### Result

```text
Matched = true
Quantity = 60
Execution Price = $100
```

The `TradeExecution` module can now execute this trade.

---

# 36. Example: Failed Match Due to Price

Suppose:

```text
Buy Price  = $95
Sell Price = $100
```

The condition:

```cpp
buyOrder.getPrice() < sellOrder.getPrice()
```

becomes:

```text
95 < 100
```

Therefore:

```cpp
return false;
```

The orders are not matched.

---

# 37. Example: Failed Match Due to Same User

Suppose:

```text
Buy User ID  = 101
Sell User ID = 101
```

The module detects:

```cpp
buyOrder.getUserId() == sellOrder.getUserId()
```

and returns:

```cpp
false;
```

This prevents self-matching.

---

# 38. Example: Failed Match Due to Different Stock

```text
Buy Symbol  = AAPL
Sell Symbol = MSFT
```

The symbols do not match, so:

```cpp
return false;
```

No trade is created.

---

# 39. Example: Partially Filled Match

Suppose:

```text
Buy Remaining  = 100
Sell Remaining = 40
```

The matching quantity is:

```text
40
```

After execution:

```text
Buy Remaining  = 60
Sell Remaining = 0
```

The expected order states after execution are:

```text
Buy  → PARTIALLY_FILLED
Sell → FILLED
```

The actual status update is handled by the `TradeExecution` module.

---

# 40. Important Design Characteristics

### 40.1 Automatic Matching

The module does not require a broker to approve a trade.

If the conditions are satisfied:

```text
Buy Price >= Sell Price
```

and all other validation checks pass, the orders are considered matched automatically.

---

### 40.2 Self-Trade Prevention

Orders belonging to the same user cannot match.

```text
Buyer ID != Seller ID
```

This is explicitly enforced by `isMatch()`.

---

### 40.3 Partial Matching Support

Because the matched quantity is:

```text
min(Buy Remaining, Sell Remaining)
```

the module naturally supports partial order execution.

---

### 40.4 First-Match Return

`matchOrders()` returns immediately after finding the first valid pair:

```cpp
return result;
```

Therefore, one call to `matchOrders()` produces at most one `MatchResult`.

---

# 41. Function Reference

| Function                     | Visibility                | Purpose                                      |
| ---------------------------- | ------------------------- | -------------------------------------------- |
| `MatchMaking()`              | Public                    | Constructs the matching engine               |
| `isMatch()`                  | Public/declared in header | Checks whether two orders can match          |
| `calculateMatchedQuantity()` | Public/declared in header | Calculates executable quantity               |
| `determineExecutionPrice()`  | Public/declared in header | Determines execution price                   |
| `matchOrders()`              | Public/declared in header | Searches order collections for a valid match |

---

# 42. Matching Rules Summary

| Condition        | Requirement                                    |
| ---------------- | ---------------------------------------------- |
| Stock            | Buy and sell symbols must be identical         |
| Users            | Buyer and seller must be different             |
| Price            | Buy price must be >= sell price                |
| Buy status       | Must not be `CANCELLED`, `FILLED`, or `FAILED` |
| Sell status      | Must not be `CANCELLED`, `FILLED`, or `FAILED` |
| Buy quantity     | Remaining quantity must be > 0                 |
| Sell quantity    | Remaining quantity must be > 0                 |
| Matched quantity | Minimum of the two remaining quantities        |
| Execution price  | Seller's order price                           |

---

# 43. Data Flow

```text
                BUY ORDER
                    |
                    v
             +-------------+
             |             |
             | MatchMaking |
             |             |
             +-------------+
                    ^
                    |
                    |
                SELL ORDER
                    |
                    v
             Validation
                    |
        +-----------+-----------+
        |           |           |
     Symbol       Price       Status
        |           |           |
        +-----------+-----------+
                    |
                    v
               Quantity
                    |
                    v
              Valid Match
                    |
        +-----------+-----------+
        |           |           |
        v           v           v
      Buy        Sell       Quantity
      Order      Order      Matched
        \          |          /
         \         |         /
          +--------+--------+
                   |
                   v
          Execution Price
                   |
                   v
             MatchResult
                   |
                   v
           TradeExecution
```

---

# 44. Module Responsibility Boundary

The MatchMaking module should be understood as a **matching decision component**, not a settlement component.

### MatchMaking handles:

```text
✓ Order compatibility
✓ Price compatibility
✓ User validation
✓ Quantity calculation
✓ Execution price determination
✓ Match result creation
```

### MatchMaking does not handle:

```text
✗ Portfolio updates
✗ Collateral deduction
✗ Collateral deposits
✗ Database transaction saving
✗ Order status persistence
✗ Actual share transfer
```

These responsibilities belong to the TradeExecution and Database modules.

---

# 45. Overall Architecture

The MatchMaking module fits into the TMS architecture as follows:

```text
                       Trading System
                             |
                             v
                         OrderBook
                             |
                    +--------+--------+
                    |                 |
                    v                 v
                Buy Orders        Sell Orders
                    |                 |
                    +--------+--------+
                             |
                             v
                       MatchMaking
                             |
                    +--------+--------+
                    |                 |
                 No Match          Match
                    |                 |
                    v                 v
                  Wait          MatchResult
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
```

---

# 46. Summary

The **MatchMaking Module** is responsible for automatically identifying compatible buy and sell orders.

It ensures that:

```text
Same Stock
    +
Different Users
    +
Buy Price >= Sell Price
    +
Active Orders
    +
Available Quantity
```

are all satisfied before a match is returned.

The matching process is:

```text
Buy Orders
    +
Sell Orders
    |
    v
isMatch()
    |
    v
Calculate Matched Quantity
    |
    v
Determine Execution Price
    |
    v
Create MatchResult
    |
    v
TradeExecution
```

The module therefore acts as the bridge between the **OrderBook** and **TradeExecution** components of the Trading Management System.

```text
OrderBook
    ↓
MatchMaking
    ↓
MatchResult
    ↓
TradeExecution
    ↓
Portfolio + Collateral + Database
```
