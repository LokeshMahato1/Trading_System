#include "Config.hpp"

#include <fstream>
#include <iostream>

std::unordered_map<std::string, std::string> Config::data;

namespace {
    std::string trim(const std::string text)
    {
        const auto begin = text.find_first_not_of(" \t");
        if (begin == std::string::npos) {
            return "";
        }
        const auto end = text.find_last_not_of(" \t");
        return text.substr(begin, end - begin + 1);
    }

    std::string stripQuotes(const std::string text)
    {
        if (text.size() >= 2)
        {
            const char first=text.front();
            const char last=text.back();

            if ((first == '"' && last == '"') || (first == '\'' && last == '\''))
            {
                return text.substr(1, text.size() - 2);
            }
        }
        return text;

    }
}

bool Config::load(const std::string &file)
{
    std::ifstream configFile(file);
    
    if(!configFile.is_open())
    {
        std::cerr << "Failed to open config file: " << file << std::endl;
        return false;
    }

    std::string line;

    while (std::getline(configFile, line))
    {
        const std::string trimmedLine = trim(line);

        if (trimmedLine.empty() || trimmedLine[0] == '#')
        {
            continue; // Skip empty lines and comments
        }

        const std::size_t delimiterPos = trimmedLine.find('=');
        if (delimiterPos == std::string::npos)
        {
            continue; // Skip invalid lines
        }

        const std::string key = trim(trimmedLine.substr(0, delimiterPos));
        std::string value = stripQuotes(trim(trimmedLine.substr(delimiterPos + 1)));
        value = stripQuotes(value); // Remove quotes if present

        if (!key.empty())
        {
            data[key] = value;
        }
    }
    return true;
}

std::string Config::get(const std::string& key, const std::string& defaultValue)
{
    const auto it = data.find(key);
    if (it == data.end()) {
        return defaultValue;
    }
    return it->second;
}

bool Config::has(const std::string& key)
{
    return data.find(key) != data.end();
}