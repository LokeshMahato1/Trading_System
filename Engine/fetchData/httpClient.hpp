#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <string>



// model of the HttpClient class
class HttpClient{
    public:
        // by default constructor
        HttpClient();

        // how the url is passed via address
        std::string get(const std::string& baseurl, const std::string& symbol);
};
#endif