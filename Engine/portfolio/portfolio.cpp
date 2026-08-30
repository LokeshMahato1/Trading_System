#include "portfolio.h"
#include <iostream>



Portfolio::Portfolio(int clientId) : clientId(clientId) {}

int Portfolio::getClientId() const {
    return clientId;
}

int Portfolio::getQuantity(const std::string& symbol) const {
    auto it = holdings.find(symbol);
    if (it == holdings.end()) {
        return 0; // If the stock is not found, return 0
    }
    return it->second.quantity;
}

double Portfolio::getAveragePrice(const std::string& symbol) const {
    auto it = holdings.find(symbol);
    if (it == holdings.end()) {
        return 0.0; // If the stock is not found, return 0.0
    }
    return it->second.averagePrice;
}

bool Portfolio:: hasStock(const std::string& symbol) const {
    return holdings.find(symbol) != holdings.end();
}



bool Portfolio::buyStock(
    const std::string& symbol,
    int quantity,
    double price)
{
    if (quantity <= 0 || price <= 0)
    {
        return false;
    }

    auto it = holdings.find(symbol);

    if (it == holdings.end())
    {
        Holding holding;

        holding.quantity = quantity;
        holding.averagePrice = price;

        holdings[symbol] = holding;
    }
    else
    {
        Holding& holding = it->second;

        double totalCost =
            (holding.quantity * holding.averagePrice)
            + (quantity * price);

        holding.quantity += quantity;

        holding.averagePrice =
            totalCost / holding.quantity;
    }

    return true;
}


bool Portfolio::sellStock(const std::string& symbol, int quantity) {

    if (quantity <= 0) {
        return false; // Invalid quantity
    }
    auto it = holdings.find(symbol);
    if (it == holdings.end()) {
        return false; // Stock not found or invalid quantity
    }

    Holding& holding = it->second;
    if (holding.quantity < quantity) {
        return false; // Not enough stock to sell
    }

    holding.quantity -= quantity;

    if (holding.quantity == 0) {
        holdings.erase(it); // Remove the stock from the portfolio if quantity is zero
    }

    return true;
}

bool Portfolio::setHolding(const std::string& symbol, int quantity, double averagePrice)
{
    if (symbol.empty() || quantity < 0 || averagePrice < 0.0) {
        return false;
    }

    if (quantity == 0) {
        holdings.erase(symbol);
        return true;
    }

    holdings[symbol] = Holding{averagePrice, quantity};
    return true;
}

void Portfolio::clear() {
    holdings.clear();
}

void Portfolio::displayPortfolio() const {
    std::cout << "Portfolio for Client ID: " << clientId << std::endl;
    std::cout << "----------------------------------------" << std::endl;;

    if(holdings.empty()) {
        std::cout << "No holdings in the portfolio." << std::endl;
        return;
    } 

    for (const auto& [symbol, holding] : holdings) {
        
        std::cout << "Stock: " << symbol << std::endl;
        std::cout << "Quantity: " << holding.quantity << std::endl;
        std::cout << "Average Price: $" << holding.averagePrice << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }


}