#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <unordered_map>


class Config{
    // how the config fill be returned -> in form of map..
    private:
        static std::unordered_map<std::string, std::string> data;
    public:
        static bool load(const std::string &file);// load the config file
        static std::string get(const std::string& key, const std::string& defaultValue = "");//return the value of the key if it exists, otherwise return the default value
        static bool has(const std::string &key);//return true if the key exists in the config, otherwise return false
};

#endif