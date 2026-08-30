#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include<map>
#include<string>


struct Holding {
    double averagePrice;
    int quantity;
};
class Portfolio
{
private:
    int clientId;
    std::map<std::string, Holding> holdings; // symbol -> Holding
public:
    Portfolio(int clientId);

    //getters
    int getClientId() const;
    int getQuantity(const std::string& symbol) const;

    double getAveragePrice(const std::string& symbol) const;

    bool hasStock(const std::string& symbol) const;

    bool buyStock(const std::string& symbol, int quantity, double price);

    bool sellStock(const std::string& symbol, int quantity);

    bool setHolding(const std::string& symbol, int quantity, double averagePrice);

    void clear();
    //Display function
    void displayPortfolio() const;
};

#endif