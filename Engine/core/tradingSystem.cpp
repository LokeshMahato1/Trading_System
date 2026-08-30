#include "tradingSystem.h"
#include "../users/admin.h"
#include "../users/client.h"
#include "../util/consoleInput.hpp"

#include <cctype>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>

namespace {

void printUsers(Database& db)
{
    std::vector<std::tuple<int, std::string, std::string, std::string>> users;
    if (!db.loadUsers(users)) {
        std::cout << "Failed to load users." << std::endl;
        return;
    }

    if (users.empty()) {
        std::cout << "No users found." << std::endl;
        return;
    }

    std::cout << "ID\tName\tUsername\tRole" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (const auto& user : users) {
        std::cout
            << std::get<0>(user) << '\t'
            << std::get<1>(user) << '\t'
            << std::get<2>(user) << '\t'
            << std::get<3>(user) << std::endl;
    }
}

void printOrders(Database& db)
{
    std::vector<Order> orders;
    if (!db.loadOrders(orders)) {
        std::cout << "Failed to load orders." << std::endl;
        return;
    }

    if (orders.empty()) {
        std::cout << "No open orders found." << std::endl;
        return;
    }

    std::cout << "OrderID\tUserID\tSymbol\tQty\tRemaining\tPrice\tType\tStatus" << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    for (const auto& order : orders) {
        std::cout
            << order.getOrderId() << '\t'
            << order.getUserId() << '\t'
            << order.getStockSymbol() << '\t'
            << order.getQuantity() << '\t'
            << order.getRemainingQuantity() << '\t'
            << order.getPrice() << '\t'
            << static_cast<int>(order.getType()) << '\t'
            << static_cast<int>(order.getStatus()) << std::endl;
    }
}

void bootstrapFirstAdmin(Database& db)
{
    if (db.hasAnyUsers()) {
        return;
    }

    std::cout << std::endl;
    std::cout << "No users found in the database yet." << std::endl;
    std::cout << "Let's create the first administrator account." << std::endl;

    std::string name;
    std::string username;
    std::string password;

    while (username.empty()) {
        std::cout << "Admin name: ";
        std::cin >> name;
        std::cout << "Admin username: ";
        std::cin >> username;
        std::cout << "Admin password: ";
        password = consoleInput::readPassword();

        if (name.empty() || username.empty() || password.empty()) {
            std::cout << "Name, username, and password cannot be empty. Please try again." << std::endl;
            username.clear();
            continue;
        }

        if (!db.addUser(name, username, password, "ADMIN")) {
            std::cout << "Failed to create admin account (username may already exist). Please try again." << std::endl;
            username.clear();
        }
    }

    std::cout << "Admin account '" << username << "' created successfully. You can log in with it now." << std::endl;
    std::cout << std::endl;
}

} // namespace

TradingSystem::TradingSystem()
        : db(),
            market(),
            tradeExecution(db),
            matchmaking()
            {}

bool TradingSystem::initializeSystem()
{
    std::cout << "Initializing Trading System..." << std::endl;

    //create database connection

    if(!db.createTables()){
        std::cerr << "Failed to create database tables." << std::endl;
        return false;
    }
    std::cout << "Database tables created successfully." << std::endl;

    bootstrapFirstAdmin(db);


    if(!market.loadStocks()){
        std::cerr << "Warning: could not load stock data (no cached data and no live fetch available)."
                   << " Set API_KEY in .env to fetch live quotes. Continuing without market data."
                   << std::endl;
    }
    else {
        std::cout << "Stock data loaded successfully." << std::endl;
    }
    
    
    std::cout << "Trading System initialized successfully." << std::endl;
    return true;
}

bool TradingSystem::loadClientState(int clientId, Portfolio& portfolio, Collateral& collateral)
{
    std::vector<std::tuple<std::string, int, double>> holdings;
    if (!db.loadPortfolioDetailed(clientId, holdings)) {
        return false;
    }

    for (const auto& entry : holdings) {
        const std::string& symbol = std::get<0>(entry);
        const int quantity = std::get<1>(entry);
        const double averagePrice = std::get<2>(entry);
        portfolio.setHolding(symbol, quantity, averagePrice);
    }

    double available = 0.0;
    double blocked = 0.0;
    double total = 0.0;
    if (!db.loadCollateral(clientId, available, blocked, total)) {
        return true;
    }

    if (available > 0.0) {
        collateral.deposit(available);
    }

    if (blocked > 0.0) {
        collateral.deposit(blocked);
        collateral.reserve(blocked);
    }

    return true;
}

bool TradingSystem::runMatchingEngine()
{
    std::vector<Order> databaseOrders;
    if (!db.loadOrders(databaseOrders)) {
        return false;
    }

    orderBook.clear();
    for (const auto& order : databaseOrders) {
        if (order.getStatus() == OrderStatus::PENDING
            || order.getStatus() == OrderStatus::PARTIALLY_FILLED) {
            orderBook.addOrder(order);
        }
    }

    std::vector<Order> buyOrders = orderBook.getBuyOrders();
    std::vector<Order> sellOrders = orderBook.getSellOrders();

    bool executedAnyTrade = false;

    while (true) {
        MatchResult match = matchmaking.matchOrders(buyOrders, sellOrders);
        if (!match.matched || match.buyOrder == nullptr || match.sellOrder == nullptr) {
            break;
        }

        Portfolio buyerPortfolio(match.buyOrder->getUserId());
        Portfolio sellerPortfolio(match.sellOrder->getUserId());
        Collateral buyerCollateral(match.buyOrder->getUserId());
        Collateral sellerCollateral(match.sellOrder->getUserId());

        if (!loadClientState(match.buyOrder->getUserId(), buyerPortfolio, buyerCollateral)
            || !loadClientState(match.sellOrder->getUserId(), sellerPortfolio, sellerCollateral)) {
            return false;
        }

        const bool executed = tradeExecution.executeTrade(
            *match.buyOrder,
            *match.sellOrder,
            buyerPortfolio,
            sellerPortfolio,
            buyerCollateral,
            sellerCollateral
        );

        if (!executed) {
            match.buyOrder->setStatus(OrderStatus::FAILED);
            match.sellOrder->setStatus(OrderStatus::FAILED);
            db.updateOrderStatus(match.buyOrder->getOrderId(), OrderStatus::FAILED, match.buyOrder->getRemainingQuantity());
            db.updateOrderStatus(match.sellOrder->getOrderId(), OrderStatus::FAILED, match.sellOrder->getRemainingQuantity());
            continue;
        }

        executedAnyTrade = true;
    }

    return executedAnyTrade;
}

void TradingSystem::runAdminSession()
{
    if (!currentUser) {
        return;
    }

    bool sessionActive = true;
    while (sessionActive) {
        currentUser->displayMenu();
        std::cout << "Enter your choice: ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number." << std::endl;
            continue;
        }

        switch (choice) {
            case 1:
                printUsers(db);
                break;
            case 2: {
                std::string name;
                std::string username;
                std::string password;
                std::string role;

                std::cout << "Name: ";
                std::cin >> name;
                std::cout << "Username: ";
                std::cin >> username;
                std::cout << "Password: ";
                password = consoleInput::readPassword();
                std::cout << "Role (ADMIN/CLIENT): ";
                std::cin >> role;

                for (char& c : role) {
                    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                }

                if (role != "ADMIN" && role != "CLIENT") {
                    std::cout << "Invalid role. Must be ADMIN or CLIENT." << std::endl;
                    break;
                }

                Admin* admin = dynamic_cast<Admin*>(currentUser.get());
                if (admin != nullptr && admin->addUser(db, name, username, password, role)) {
                    std::cout << "User added successfully." << std::endl;
                }
                else {
                    std::cout << "Failed to add user." << std::endl;
                }
                break;
            }
            case 3: {
                int userId = 0;
                std::cout << "User ID to remove: ";
                std::cin >> userId;

                Admin* admin = dynamic_cast<Admin*>(currentUser.get());
                if (admin != nullptr && admin->removeUser(db, userId)) {
                    std::cout << "User removed successfully." << std::endl;
                }
                else {
                    std::cout << "Failed to remove user." << std::endl;
                }
                break;
            }
            case 4: {
                int userId = 0;
                std::string newPassword;

                std::cout << "User ID: ";
                std::cin >> userId;
                std::cout << "New password: ";
                newPassword = consoleInput::readPassword();

                Admin* admin = dynamic_cast<Admin*>(currentUser.get());
                if (admin != nullptr && admin->resetPassword(db, userId, newPassword)) {
                    std::cout << "Password reset successfully." << std::endl;
                }
                else {
                    std::cout << "Failed to reset password." << std::endl;
                }
                break;
            }
            case 5:
                printOrders(db);
                break;
            case 6:
                market.setMarketStatus(true);
                std::cout << "Market opened." << std::endl;
                break;
            case 7:
                market.setMarketStatus(false);
                std::cout << "Market closed." << std::endl;
                break;
            case 8:
                currentUser->logout();
                currentUser.reset();
                sessionActive = false;
                std::cout << "Logged out successfully." << std::endl;
                break;
            default:
                std::cout << "Option not implemented yet." << std::endl;
                break;
        }
    }
}

void TradingSystem::runClientSession()
{
    if (!currentUser) {
        return;
    }

    Client* client = dynamic_cast<Client*>(currentUser.get());
    if (client == nullptr) {
        return;
    }

    bool sessionActive = true;
    while (sessionActive) {
        currentUser->displayMenu();
        std::cout << "Enter your choice: ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number." << std::endl;
            continue;
        }

        switch (choice) {
            case 1:
                client->viewPortfolio();
                break;
            case 2: {
                std::string symbol;
                int quantity = 0;
                double price = 0.0;

                std::cout << "Symbol: ";
                std::cin >> symbol;
                std::cout << "Quantity: ";
                std::cin >> quantity;
                std::cout << "Price: ";
                std::cin >> price;

                if (client->placeBuyOrder(symbol, quantity, price)) {
                    std::cout << "Buy order placed successfully." << std::endl;
                }
                else {
                    std::cout << "Failed to place buy order." << std::endl;
                }
                break;
            }
            case 3: {
                std::string symbol;
                int quantity = 0;
                double price = 0.0;

                std::cout << "Symbol: ";
                std::cin >> symbol;
                std::cout << "Quantity: ";
                std::cin >> quantity;
                std::cout << "Price: ";
                std::cin >> price;

                if (client->placeSellOrder(symbol, quantity, price)) {
                    std::cout << "Sell order placed successfully." << std::endl;
                }
                else {
                    std::cout << "Failed to place sell order." << std::endl;
                }
                break;
            }
            case 4:
                client->viewCollateral();
                break;
            case 5: {
                double amount = 0.0;
                std::cout << "Deposit amount: ";
                std::cin >> amount;

                if (client->depositCollateral(amount)) {
                    std::cout << "Collateral deposited successfully." << std::endl;
                }
                else {
                    std::cout << "Failed to deposit collateral." << std::endl;
                }
                break;
            }
            case 6: {
                double amount = 0.0;
                std::cout << "Withdraw amount: ";
                std::cin >> amount;

                if (client->withdrawCollateral(amount)) {
                    std::cout << "Collateral withdrawn successfully." << std::endl;
                }
                else {
                    std::cout << "Failed to withdraw collateral." << std::endl;
                }
                break;
            }
            case 7: {
                int orderId = 0;
                int newQuantity = 0;
                double newPrice = 0.0;

                std::cout << "Order ID: ";
                std::cin >> orderId;
                std::cout << "New quantity: ";
                std::cin >> newQuantity;
                std::cout << "New price: ";
                std::cin >> newPrice;

                if (client->modifyOrder(orderId, newQuantity, newPrice)) {
                    std::cout << "Order modified successfully." << std::endl;
                }
                else {
                    std::cout << "Failed to modify order." << std::endl;
                }
                break;
            }
            case 8: {
                int orderId = 0;
                std::cout << "Order ID: ";
                std::cin >> orderId;

                if (client->cancelOrder(orderId)) {
                    std::cout << "Order cancelled successfully." << std::endl;
                }
                else {
                    std::cout << "Failed to cancel order." << std::endl;
                }
                break;
            }
            case 9:
                currentUser->logout();
                currentUser.reset();
                sessionActive = false;
                std::cout << "Logged out successfully." << std::endl;
                break;
            default:
                std::cout << "Option not implemented yet." << std::endl;
                break;
        }
    }
}

void TradingSystem::run()
{
    std::cout << "Running Trading System..." << std::endl;
    while (true) {
        if (!runMatchingEngine()) {
            // No executable match found or no open orders; continue menu flow.
        }

        std::cout << std::endl;
        std::cout<<"======Main Menu======"<<std::endl;
        std::cout<<"1. Login"<<std::endl;
        std::cout<<"2. Exit"<<std::endl;
        std::cout<<"Enter your choice: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number." << std::endl;
            continue;
        }

        switch (choice) {
            case 1:
                login();
                break;
            case 2:
                shutdown();
                return;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }

}

void TradingSystem::login()
{
    std::string username;
    std::string password;

    std::cout << "Username: ";
    std::cin >> username;

    std::cout << "Password: ";
    password = consoleInput::readPassword();

    std::string role;
    int userId = 0;

    if (!db.verifyUser(username, password, role, userId)) {
        std::cout << "\nInvalid username or password.\n";
        return;
    }

    if (role == "ADMIN" || role == "admin") {
        currentUser = std::make_unique<Admin>(
            userId,
            "Administrator",
            username,
            password
        );
    }
    else {
        currentUser = std::make_unique<Client>(
            userId,
            "Client",
            username,
            password
        );
    }

    if (!currentUser->login(username, password)) {
        currentUser.reset();
        std::cout << "\nLogin failed.\n" << std::endl;
        return;
    }

    std::cout << "\nLogin Successful!\n";

    if (currentUser->isAdmin()) {
        runAdminSession();
    }
    else {
        runClientSession();
    }
}

void TradingSystem::shutdown()
{
    std::cout << "Shutting down Trading System..." << std::endl;
    currentUser.reset();
    db.close();
    std::cout << "Trading System shut down successfully." << std::endl;
}
