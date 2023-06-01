#pragma once
#include <sstream>
#include <vector>
#include <string>

namespace redis
{
    struct utility
    {
        /* split string by blank character base on stringstream */
        static auto str_split(const std::string &s) -> std::vector<std::string>
        {
            std::vector<std::string> res;
            std::stringstream ss(s);
            std::string word;
            while (ss >> word) {
                res.emplace_back(std::move(word));
            }
            return res;
        }
    };
} // namespace redis
