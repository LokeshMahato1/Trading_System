#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>

struct StockData
{
    // Price Information
    double change = 0.0;
    double close = 0.0;
    double high = 0.0;
    double low = 0.0;
    double openingPrice = 0.0;
    double previousClose = 0.0;
    double percentChange = 0.0;

    // Company Information
    std::string symbol;
    std::string companyName;
    std::string exchange;
    std::string currency;
    std::string date;

    // Market Status
    // Keep both naming styles to remain compatible with existing code paths.
    union
    {
        bool isMarketOpen;
        bool is_market_open;
    };

    // Market Statistics
    int timestamp = 0;
    int volume = 0;

    StockData()
        : isMarketOpen(false)
    {
    }
};

#endif