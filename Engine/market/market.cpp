#include "market.h"

#include "../database/db.hpp"
#include "../fetchData/httpClient.hpp"

#include <chrono>
#include <cctype>
#include <iostream>
#include <thread>
#include <vector>

namespace {

std::string trim(const std::string& text)
{
    const auto begin = text.find_first_not_of(" \t\n\r");
    if (begin == std::string::npos) {
        return {};
    }

    const auto end = text.find_last_not_of(" \t\n\r");
    return text.substr(begin, end - begin + 1);
}

std::string extractRawField(const std::string& response, const std::string& key)
{
    const std::string needle = '"' + key + '"';
    const std::size_t keyPosition = response.find(needle);
    if (keyPosition == std::string::npos) {
        return {};
    }

    std::size_t valueStart = response.find(':', keyPosition + needle.size());
    if (valueStart == std::string::npos) {
        return {};
    }

    ++valueStart;
    while (valueStart < response.size() && std::isspace(static_cast<unsigned char>(response[valueStart]))) {
        ++valueStart;
    }

    if (valueStart >= response.size()) {
        return {};
    }

    if (response[valueStart] == '"') {
        ++valueStart;
        const std::size_t valueEnd = response.find('"', valueStart);
        if (valueEnd == std::string::npos) {
            return {};
        }
        return response.substr(valueStart, valueEnd - valueStart);
    }

    std::size_t valueEnd = valueStart;
    while (valueEnd < response.size()) {
        const char current = response[valueEnd];
        if (current == ',' || current == '}') {
            break;
        }
        ++valueEnd;
    }

    return trim(response.substr(valueStart, valueEnd - valueStart));
}

double extractDoubleField(const std::string& response, const std::string& key)
{
    const std::string value = extractRawField(response, key);
    if (value.empty() || value == "null") {
        return 0.0;
    }

    try {
        return std::stod(value);
    }
    catch (...) {
        return 0.0;
    }
}

int extractIntField(const std::string& response, const std::string& key)
{
    const std::string value = extractRawField(response, key);
    if (value.empty() || value == "null") {
        return 0;
    }

    try {
        return std::stoi(value);
    }
    catch (...) {
        return 0;
    }
}

bool extractBoolField(const std::string& response, const std::string& key)
{
    const std::string value = extractRawField(response, key);
    return value == "true" || value == "1";
}

} // namespace

Market::Market()
    : marketOpen(false)
{
}

void Market::updateStock(const StockData& stock)
{
    if (stock.symbol.empty()) {
        return;
    }

    stocks[stock.symbol] = stock;
}

bool Market::stockExists(const std::string& symbol) const
{
    return stocks.find(symbol) != stocks.end();
}

StockData Market::getStock(const std::string& symbol) const
{
    const auto found = stocks.find(symbol);
    if (found == stocks.end()) {
        return StockData();
    }

    return found->second;
}

double Market::getCurrentPrice(const std::string& symbol) const
{
    const auto found = stocks.find(symbol);
    if (found == stocks.end()) {
        return 0.0;
    }

    return found->second.close;
}

const std::unordered_map<std::string, StockData>& Market::getAllStocks() const
{
    return stocks;
}

void Market::displayStocks() const
{
    if (stocks.empty()) {
        std::cout << "No stocks available.\n";
        return;
    }

    std::cout << "-------------------------------------------------------------\n";
    std::cout << "Symbol\tCompany\t\tPrice\tVolume\n";
    std::cout << "-------------------------------------------------------------\n";

    for (const auto& stock : stocks) {
        std::cout
            << stock.second.symbol << "\t"
            << stock.second.companyName << "\t"
            << stock.second.close << "\t"
            << stock.second.volume << '\n';
    }
}

bool Market::isMarketOpen() const
{
    return marketOpen;
}

void Market::setMarketStatus(bool status)
{
    marketOpen = status;
}

bool Market::loadStocksFromApi()
{
    HttpClient client;
    const std::string url = "https://api.twelvedata.com";
    const std::string symbols[] = {"AAPL", "GOOGL", "AMZN", "MSFT", "TSLA", "META", "NVDA", "NFLX"};

    std::vector<StockData> fetchedStocks;

    for (const auto& symbol : symbols) {
        const std::string response = client.get(url, symbol);
        if (response.empty()) {
            std::cerr << "Empty response while loading " << symbol << std::endl;
            continue;
        }

        StockData stock;

        try {
            stock.change = extractDoubleField(response, "change");
            stock.close = extractDoubleField(response, "close");
            stock.high = extractDoubleField(response, "high");
            stock.low = extractDoubleField(response, "low");
            stock.openingPrice = extractDoubleField(response, "open");
            stock.previousClose = extractDoubleField(response, "previous_close");
            stock.percentChange = extractDoubleField(response, "percent_change");

            stock.currency = extractRawField(response, "currency");
            stock.date = extractRawField(response, "datetime");
            stock.exchange = extractRawField(response, "exchange");
            stock.companyName = extractRawField(response, "name");

            const std::string responseSymbol = extractRawField(response, "symbol");
            stock.symbol = responseSymbol.empty() ? symbol : responseSymbol;
            stock.isMarketOpen = extractBoolField(response, "is_market_open");
            stock.timestamp = extractIntField(response, "timestamp");
            stock.volume = extractIntField(response, "volume");

            updateStock(stock);
            fetchedStocks.push_back(stock);
        }
        catch (const std::exception& error) {
            std::cerr << "Failed to load " << symbol << ": " << error.what() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    if (fetchedStocks.empty()) {
        return false;
    }

    Database db;
    if (!db.addStocks(fetchedStocks)) {
        std::cerr << "Failed to save stocks to database.\n";
    }

    marketOpen = true;
    return true;
}

bool Market::loadStocks()
{
    Database db;
    std::vector<StockData> databaseStocks;

    if (db.loadStocks(databaseStocks) && !databaseStocks.empty()) {
        for (const auto& stock : databaseStocks) {
            updateStock(stock);
        }
        marketOpen = true;
        return true;
    }

    const bool loadedFromApi = loadStocksFromApi();
    marketOpen = loadedFromApi;
    return loadedFromApi;
}

bool Market::refreshMarket()
{
    stocks.clear();
    const bool refreshed = loadStocksFromApi();
    marketOpen = refreshed;
    return refreshed;
}
