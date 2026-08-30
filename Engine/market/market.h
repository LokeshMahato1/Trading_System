#ifndef MARKET_H
#define MARKET_H

#include <string>
#include <unordered_map>
#include <vector>

#include "../model/models.hpp"

class Market {
private:
    std::unordered_map<std::string, StockData> stocks;
    bool marketOpen;

    bool loadStocksFromApi();

public:
    Market();

    // Update in-memory stock information.
    void updateStock(const StockData& stock);

    // Check whether a symbol exists in memory.
    bool stockExists(const std::string& symbol) const;

    // Return stock details for a symbol.
    StockData getStock(const std::string& symbol) const;

    // Return current stock close price for a symbol.
    double getCurrentPrice(const std::string& symbol) const;

    // Return all loaded stocks.
    const std::unordered_map<std::string, StockData>& getAllStocks() const;

    // Display loaded stocks in tabular form.
    void displayStocks() const;

    // Market status controls.
    bool isMarketOpen() const;
    void setMarketStatus(bool status);

    // Load stock data from database first, then API fallback if needed.
    bool loadStocks();

    // Refresh in-memory stocks by fetching latest values from API.
    bool refreshMarket();
};

#endif