// Implementation of HttpClient: fetches a quote for a symbol from the
// configured market-data API using httplib.

#include "httpClient.hpp"
#include "httplib.h"
#include "../config/Config.hpp"

#include <iostream>

HttpClient::HttpClient()
{
}

std::string HttpClient::get(const std::string& baseUrl, const std::string& symbol)
{
    const std::string apiKey = Config::get("API_KEY");

    if (apiKey.empty()) {
        std::cerr << "HttpClient: API_KEY is not set (check your .env file)." << std::endl;
        return "";
    }

    httplib::Client client(baseUrl);
    client.set_connection_timeout(5);
    client.set_read_timeout(5);

    const std::string path = "/quote?symbol=" + symbol + "&apikey=" + apiKey;

    auto response = client.Get(path);
    if (!response) {
        std::cerr << "HttpClient: request failed for " << symbol
                   << " (" << httplib::to_string(response.error()) << ")" << std::endl;
        return "";
    }

    if (response->status != 200) {
        std::cerr << "HttpClient: unexpected status " << response->status
                   << " for " << symbol << std::endl;
        return "";
    }

    return response->body;
}
