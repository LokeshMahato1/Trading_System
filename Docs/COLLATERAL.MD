# Collateral Module

## 1. Overview

The **Collateral Module** is responsible for managing the financial collateral of a client in the Trading Management System (TMS).

Collateral represents the funds available to a client for trading. The module maintains two separate balances:

* **Available Collateral** — funds that can currently be used.
* **Blocked Collateral** — funds temporarily reserved for pending orders.

The module provides operations to:

* Deposit collateral
* Withdraw collateral
* Reserve collateral for an order
* Release reserved collateral
* Deduct collateral after a successful transaction
* Retrieve collateral balances

The module is designed to work with other TMS components such as the **Order Book**, **Matchmaking**, **Trade Execution**, and **Portfolio** modules.

---

## 2. Module Responsibilities

The `Collateral` class is responsible for maintaining the client's collateral balance and ensuring that collateral cannot be overused.

### Main responsibilities

| Responsibility          | Description                                           |
| ----------------------- | ----------------------------------------------------- |
| Store client ID         | Identifies the owner of the collateral                |
| Store available balance | Tracks collateral that is currently usable            |
| Store blocked balance   | Tracks collateral reserved for pending orders         |
| Deposit                 | Adds funds to available collateral                    |
| Withdraw                | Removes funds from available collateral               |
| Reserve                 | Moves funds from available to blocked collateral      |
| Release                 | Moves blocked collateral back to available collateral |
| Deduct                  | Permanently removes blocked collateral after use      |
| Calculate total         | Returns available + blocked collateral                |

---

# 3. File Structure

The module consists of two files:

```text
Collateral/
├── collateral.h
└── collateral.cpp
```

### `collateral.h`

Contains the declaration of the `Collateral` class, including:

* Private data members
* Constructor
* Getter functions
* Collateral management operations

### `collateral.cpp`

Contains the implementation of all functions declared in `collateral.h`.

---

# 4. Class Definition

```cpp
class Collateral
{
private:
    int clientId;
    double availableCollateral;
    double blockedCollateral;

public:
    Collateral(int clientId, double initialCollateral = 0.0);

    int getClientId() const;
    double getAvailableCollateral() const;
    double getBlockedCollateral() const;
    double getTotalCollateral() const;

    bool deposit(double amount);
    bool withdraw(double amount);

    bool reserve(double amount);
    bool release(double amount);
    bool deduct(double amount);
};
```

---

# 5. Data Members

The class contains three private data members.

## 5.1 `clientId`

```cpp
int clientId;
```

Stores the unique ID of the client who owns the collateral.

Example:

```text
clientId = 101
```

This allows the system to associate the collateral with a particular client.

---

## 5.2 `availableCollateral`

```cpp
double availableCollateral;
```

Stores the amount of collateral currently available for use.

For example:

```text
Available Collateral = $10,000
Blocked Collateral   = $2,000
```

The client can only withdraw or reserve funds from the available balance.

---

## 5.3 `blockedCollateral`

```cpp
double blockedCollateral;
```

Stores collateral that has been temporarily reserved.

Blocked collateral cannot be withdrawn because it is already committed to an operation such as a pending buy order.

Example:

```text
Available Collateral = $8,000
Blocked Collateral   = $2,000
```

The `$2,000` is reserved and cannot be withdrawn until it is either released or deducted.

---

# 6. Constructor

```cpp
Collateral::Collateral(int clientId, double initialCollateral)
    : clientId(clientId),
      availableCollateral(initialCollateral),
      blockedCollateral(0.0)
{}
```

The constructor initializes a client's collateral account.

### Parameters

| Parameter           | Type     | Description               |
| ------------------- | -------- | ------------------------- |
| `clientId`          | `int`    | Unique ID of the client   |
| `initialCollateral` | `double` | Initial collateral amount |

The default value of `initialCollateral` is:

```cpp
0.0
```

Therefore, the following is valid:

```cpp
Collateral collateral(101);
```

Result:

```text
Client ID:           101
Available:           0
Blocked:             0
```

An initial balance can also be provided:

```cpp
Collateral collateral(101, 10000.0);
```

Result:

```text
Client ID:           101
Available:           10000
Blocked:             0
```

---

# 7. Getter Functions

Getter functions provide read-only access to the collateral information.

---

## 7.1 `getClientId()`

```cpp
int Collateral::getClientId() const {
    return clientId;
}
```

Returns the ID of the client associated with the collateral account.

### Example

```cpp
int id = collateral.getClientId();
```

---

## 7.2 `getAvailableCollateral()`

```cpp
double Collateral::getAvailableCollateral() const {
    return availableCollateral;
}
```

Returns the amount of collateral that is currently available.

Example:

```text
Available Collateral = 5000
```

---

## 7.3 `getBlockedCollateral()`

```cpp
double Collateral::getBlockedCollateral() const {
    return blockedCollateral;
}
```

Returns the amount of collateral currently reserved or blocked.

---

## 7.4 `getTotalCollateral()`

```cpp
double Collateral::getTotalCollateral() const {
    return availableCollateral + blockedCollateral;
}
```

Returns the client's total collateral.

The calculation is:

```text
Total Collateral
        =
Available Collateral
        +
Blocked Collateral
```

### Example

```text
Available = 7,000
Blocked   = 3,000

Total = 7,000 + 3,000
      = 10,000
```

Importantly, reserving collateral does **not** reduce total collateral. It only moves collateral from the available category to the blocked category.

---

# 8. Deposit Operation

```cpp
bool Collateral::deposit(double amount)
```

The `deposit()` function adds collateral to the client's available balance.

### Logic

```text
             Deposit
                |
                v
        Is amount > 0?
          /         \
        No           Yes
        |             |
      Fail       Add to available
        |             |
      false          true
```

### Implementation

```cpp
bool Collateral::deposit(double amount) {
    if (amount <= 0) {
        return false;
    }

    availableCollateral += amount;
    return true;
}
```

### Example

Initial state:

```text
Available = 5,000
Blocked   = 0
```

Operation:

```cpp
collateral.deposit(2000);
```

New state:

```text
Available = 7,000
Blocked   = 0
Total     = 7,000
```

### Invalid deposit

```cpp
collateral.deposit(-500);
```

Result:

```cpp
false
```

The balance remains unchanged.

---

# 9. Withdrawal Operation

```cpp
bool Collateral::withdraw(double amount)
```

The `withdraw()` function removes collateral from the available balance.

A withdrawal is only successful when:

1. The amount is greater than zero.
2. The client has sufficient available collateral.

### Logic

```text
              Withdraw
                  |
                  v
           Is amount > 0?
            /          \
          No            Yes
          |              |
        false      Is amount <= available?
                       /       \
                     No         Yes
                     |           |
                   false      Withdraw
                                 |
                                true
```

### Example

Initial state:

```text
Available = 10,000
Blocked   = 2,000
```

Operation:

```cpp
collateral.withdraw(3000);
```

Result:

```text
Available = 7,000
Blocked   = 2,000
Total     = 9,000
```

### Insufficient collateral

If:

```text
Available = 2,000
```

and the client attempts:

```cpp
collateral.withdraw(3000);
```

the operation fails:

```cpp
false
```

The balance remains:

```text
Available = 2,000
```

---

# 10. Reserve Operation

```cpp
bool Collateral::reserve(double amount)
```

The `reserve()` function moves collateral from the available balance to the blocked balance.

This is particularly important when a client places an order.

For example, suppose a client places a buy order worth:

```text
5 shares × $100 = $500
```

The system can reserve `$500` so that the client cannot use the same funds for another order.

### State transition

```text
Before reservation:

Available = 10,000
Blocked   = 0

        reserve(500)

After reservation:

Available = 9,500
Blocked   = 500
```

### Implementation

```cpp
bool Collateral::reserve(double amount) {
    if (amount <= 0) {
        return false;
    }

    if (amount > availableCollateral) {
        return false;
    }

    availableCollateral -= amount;
    blockedCollateral += amount;

    return true;
}
```

### Important rule

The system cannot reserve more than the available collateral.

For example:

```text
Available = 1,000
Reserve   = 1,500
```

The operation returns:

```cpp
false
```

---

# 11. Release Operation

```cpp
bool Collateral::release(double amount)
```

The `release()` function moves collateral from blocked collateral back into available collateral.

This is useful when a pending order is cancelled or when reserved funds are no longer required.

### State transition

```text
Before release:

Available = 7,000
Blocked   = 3,000

        release(1,000)

After release:

Available = 8,000
Blocked   = 2,000
```

### Implementation

```cpp
bool Collateral::release(double amount) {
    if (amount <= 0) {
        return false;
    }

    if (amount > blockedCollateral) {
        return false;
    }

    blockedCollateral -= amount;
    availableCollateral += amount;

    return true;
}
```

### Example use case

A client places a buy order:

```text
Order value = 5,000
```

The system reserves:

```text
5,000
```

If the order is cancelled before execution:

```cpp
collateral.release(5000);
```

The reserved collateral becomes available again.

---

# 12. Deduct Operation

```cpp
bool Collateral::deduct(double amount)
```

The `deduct()` function permanently removes an amount from blocked collateral.

Unlike `release()`, the deducted amount does **not** return to available collateral.

This operation is useful when reserved collateral has actually been consumed by a trade.

### State transition

Before deduction:

```text
Available = 5,000
Blocked   = 3,000
Total     = 8,000
```

Operation:

```cpp
collateral.deduct(2,000);
```

After deduction:

```text
Available = 5,000
Blocked   = 1,000
Total     = 6,000
```

The `$2,000` has been permanently removed from collateral.

### Implementation

```cpp
bool Collateral::deduct(double amount) {
    if (amount <= 0) {
        return false;
    }

    if (amount > blockedCollateral) {
        return false;
    }

    blockedCollateral -= amount;

    return true;
}
```

---

# 13. Reserve, Release, and Deduct Difference

These three operations have different purposes.

| Operation   | Available |   Blocked |     Total |
| ----------- | --------: | --------: | --------: |
| `reserve()` | Decreases | Increases |      Same |
| `release()` | Increases | Decreases |      Same |
| `deduct()`  |      Same | Decreases | Decreases |

### Example

Starting balance:

```text
Available = 10,000
Blocked   = 0
Total     = 10,000
```

### Reserve `$3,000`

```text
Available = 7,000
Blocked   = 3,000
Total     = 10,000
```

### Release `$1,000`

```text
Available = 8,000
Blocked   = 2,000
Total     = 10,000
```

### Deduct `$1,000`

```text
Available = 8,000
Blocked   = 1,000
Total     = 9,000
```

---

# 14. Complete Collateral Lifecycle

The typical lifecycle of collateral during trading can be represented as:

```text
                    Client
                       |
                       v
               Deposit Collateral
                       |
                       v
              Available Collateral
                       |
                       v
                 Place Order
                       |
                       v
              Reserve Collateral
                       |
                       v
              Blocked Collateral
                       |
             +---------+---------+
             |                   |
             v                   v
       Order Cancelled       Trade Executed
             |                   |
             v                   v
          Release()            Deduct()
             |                   |
             v                   v
       Available Again      Collateral Consumed
```

---

# 15. Integration with the TMS

The Collateral Module interacts primarily with the trading components.

A simplified system flow is:

```text
                    +----------------+
                    |     Client     |
                    +-------+--------+
                            |
                            v
                    +----------------+
                    |  Order Module  |
                    +-------+--------+
                            |
                            v
                    +----------------+
                    |   Collateral   |
                    |     Module     |
                    +-------+--------+
                            |
                  reserve collateral
                            |
                            v
                    +----------------+
                    |   Order Book   |
                    +-------+--------+
                            |
                            v
                    +----------------+
                    |   Matchmaking  |
                    +-------+--------+
                            |
                         Match
                            |
                            v
                    +----------------+
                    | Trade Execution|
                    +-------+--------+
                            |
                  +---------+---------+
                  |                   |
                  v                   v
              Portfolio          Collateral
              Update             Deduction
```

---

# 16. Example Usage

A basic example of using the class:

```cpp
#include "collateral.h"
#include <iostream>

int main()
{
    Collateral collateral(101, 10000.0);

    std::cout << "Available: "
              << collateral.getAvailableCollateral()
              << '\n';

    collateral.reserve(3000);

    std::cout << "Available after reserve: "
              << collateral.getAvailableCollateral()
              << '\n';

    std::cout << "Blocked: "
              << collateral.getBlockedCollateral()
              << '\n';

    collateral.release(1000);

    std::cout << "Available after release: "
              << collateral.getAvailableCollateral()
              << '\n';

    collateral.deduct(500);

    std::cout << "Blocked after deduction: "
              << collateral.getBlockedCollateral()
              << '\n';

    return 0;
}
```

Expected state transitions:

```text
Initial:
Available = 10,000
Blocked   = 0

After reserve(3,000):
Available = 7,000
Blocked   = 3,000

After release(1,000):
Available = 8,000
Blocked   = 2,000

After deduct(500):
Available = 8,000
Blocked   = 1,500
```

---

# 17. Return Values and Validation

All modification functions return a `bool`.

| Function     | `true`                 | `false`                                            |
| ------------ | ---------------------- | -------------------------------------------------- |
| `deposit()`  | Deposit successful     | Invalid amount                                     |
| `withdraw()` | Withdrawal successful  | Invalid amount / insufficient available collateral |
| `reserve()`  | Reservation successful | Invalid amount / insufficient available collateral |
| `release()`  | Release successful     | Invalid amount / insufficient blocked collateral   |
| `deduct()`   | Deduction successful   | Invalid amount / insufficient blocked collateral   |

This allows other modules to verify whether an operation succeeded before continuing.

Example:

```cpp
if (!collateral.reserve(orderValue)) {
    // Order cannot reserve sufficient collateral
}
```

---

# 18. Encapsulation

The collateral balances are declared as `private`:

```cpp
private:
    int clientId;
    double availableCollateral;
    double blockedCollateral;
```

Other classes cannot directly modify these values.

For example, this is not allowed:

```cpp
collateral.availableCollateral = 50000;
```

Instead, controlled operations must be used:

```cpp
collateral.deposit(50000);
```

This protects the internal state of the collateral account and ensures that validation rules are applied.

---

# 19. Const-Correctness

The getter functions are declared with `const`:

```cpp
double getAvailableCollateral() const;
```

This means that calling the getter does not modify the `Collateral` object.

For example:

```cpp
const Collateral collateral(101, 10000);

double balance = collateral.getAvailableCollateral();
```

The getter can safely be called on a constant object.

---

# 20. Validation Rules

The module currently applies the following rules:

### Deposit

```text
amount > 0
```

### Withdraw

```text
amount > 0
amount <= availableCollateral
```

### Reserve

```text
amount > 0
amount <= availableCollateral
```

### Release

```text
amount > 0
amount <= blockedCollateral
```

### Deduct

```text
amount > 0
amount <= blockedCollateral
```

These rules prevent negative transactions and prevent the system from using more collateral than is available.

---

# 21. State Invariants

The module maintains several important conditions.

### Available collateral

```text
availableCollateral >= 0
```

### Blocked collateral

```text
blockedCollateral >= 0
```

### Total collateral

```text
totalCollateral =
    availableCollateral + blockedCollateral
```

The class operations are designed to preserve these conditions.

---

# 22. Dependencies

The Collateral module has no external library dependencies.

It uses standard C++ language features only.

### Required

* C++ compiler
* C++ standard library if used with input/output elsewhere

No Qt, SQLite, Boost, OpenSSL, or third-party library is required by this module itself.

---

# 23. Compilation

The module can be compiled independently using:

```bash
g++ -std=c++17 collateral.cpp main.cpp -o collateral_test
```

Then run:

```bash
./collateral_test
```

If the project uses CMake, the source file should be included in the appropriate target.

---

# 24. Design Summary

The `Collateral` class provides a simple and controlled mechanism for managing client collateral.

Its most important concept is the separation between:

```text
Available Collateral
        +
Blocked Collateral
        =
Total Collateral
```

Collateral can move between available and blocked states during the lifecycle of an order.

```text
                 +----------------------+
                 | Available Collateral |
                 +----------+-----------+
                            |
                         reserve()
                            |
                            v
                 +----------------------+
                 | Blocked Collateral  |
                 +----+------------+----+
                      |            |
                release()       deduct()
                      |            |
                      v            v
                  Available      Consumed
```

This design supports the TMS requirement that collateral associated with pending orders should be protected from being reused, while still allowing it to be returned when an order is cancelled or no longer requires the reservation.

---

# 25. Function Reference

| Function                   | Purpose                                    |
| -------------------------- | ------------------------------------------ |
| `Collateral()`             | Creates a client's collateral account      |
| `getClientId()`            | Returns the client ID                      |
| `getAvailableCollateral()` | Returns usable collateral                  |
| `getBlockedCollateral()`   | Returns reserved collateral                |
| `getTotalCollateral()`     | Returns available + blocked collateral     |
| `deposit()`                | Adds collateral                            |
| `withdraw()`               | Removes available collateral               |
| `reserve()`                | Moves available collateral to blocked      |
| `release()`                | Moves blocked collateral back to available |
| `deduct()`                 | Permanently removes blocked collateral     |

---

## Conclusion

The **Collateral Module** forms an important part of the Trading Management System's financial-control layer. It prevents clients from withdrawing or reusing funds that have already been committed to orders and provides the Trade Execution system with controlled operations for consuming or releasing reserved collateral.

The module is intentionally independent of the GUI and database layers, allowing it to be tested independently and integrated with the rest of the TMS engine.
