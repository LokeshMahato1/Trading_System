# Artha Bazaar — Trading Management System

A simulated stock-trading platform with a C++ matching engine and a Qt Quick
(QML) desktop GUI. Clients deposit collateral, place buy/sell orders, and get
matched automatically against the order book; admins manage users and control
the market session. Live stock quotes are pulled from the TwelveData API,
with a local SQLite cache as a fallback.

## Features

**Client**
- Place, modify, and cancel buy/sell orders
- Deposit and withdraw collateral (funds are reserved against open buy
  orders)
- View portfolio (holdings, average price, live P&L)
- View order history and status (pending, partially filled, filled,
  cancelled, failed)

**Admin**
- Add, remove, and reset passwords for users (client or admin)
- Open/close the market session
- Refresh live market data from the API
- View all users and all orders across the system

**Engine**
- Price-time-priority order matching (order book + matching engine)
- Automated trade execution with portfolio and collateral settlement
- SQLite-backed persistence for users, orders, portfolios, collateral, and
  transactions
- Salted SHA-256 password hashing

## Project Structure

```
Trading_System/
├── Engine/                    # Core trading engine (no GUI dependencies)
│   ├── core/                  # TradingSystem orchestration (CLI entry logic)
│   ├── users/                 # User, Admin, Client
│   ├── order/                 # Order
│   ├── orderbook/             # OrderBook
│   ├── Matchmaking/           # Matching engine
│   ├── execution/             # Trade execution & settlement
│   ├── portfolio/             # Portfolio
│   ├── collateral/            # Collateral
│   ├── market/                # Market data (live + cached)
│   ├── database/              # SQLite persistence layer
│   ├── fetchData/             # HTTP client for the market data API
│   ├── config/                # .env config loader
│   └── util/                  # Password hashing, console helpers
├── Gui/                       # Qt Quick (QML) desktop front-end
│   ├── Bridge.h / Bridge.cpp  # Connects QML to the Engine
│   ├── main.cpp               # App entry point
│   └── *.qml                  # Login, Dashboard, Admin, and dialog screens
├── CMakeLists.txt
├── .env                       # API_KEY (not committed — see Configuration)
└── Trading_Management_System.db  # SQLite database (created on first run)
```

## Requirements

- CMake ≥ 3.16
- A C++20 compiler (GCC, Clang, or MSVC)
- Qt 6.5+ with the **Quick** and **QuickControls2** modules
- SQLite3 development libraries
- OpenSSL (optional — enables HTTPS for live market-data fetches; without
  it the app still runs, but live quote refresh will fail)
- A [TwelveData](https://twelvedata.com/) API key (free tier works) for live
  market data

On Debian/Ubuntu, the non-Qt dependencies can be installed with:

```bash
sudo apt install build-essential cmake libsqlite3-dev libssl-dev
```

Qt 6 is typically installed via the [Qt online installer](https://www.qt.io/download-qt-installer)
or your distro's `qt6-*` packages (`qt6-base-dev`, `qt6-declarative-dev`, etc).

## Configuration

Create a `.env` file in the project root (this file is git-ignored and
should never be committed):

```
API_KEY=your_twelvedata_api_key_here
```

If no API key is set, the app falls back to whatever market data is already
cached in the local SQLite database.

## Build & Run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run the GUI from the **project root** (both `.env` and the SQLite database
are read/created using paths relative to the working directory):

```bash
./build/trading_gui
```

On first run, with no users in the database, the login screen prompts you to
create the initial administrator account. From there you can log in as that
admin to add client accounts.

## Notes

- The SQLite database file (`Trading_Management_System.db`) is created
  automatically on first run and persists all users, orders, portfolios,
  collateral, and transaction history between sessions.
- Passwords are stored as salted SHA-256 hashes, never in plaintext, in the
  database.
- The market composite index shown in the dashboard is a synthetic value
  derived from the average percent change across listed stocks — it is not
  a real exchange index.