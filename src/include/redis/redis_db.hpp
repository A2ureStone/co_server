#pragma once
#include <unordered_map>
#include <unordered_set>
#include "redis/redis_object.hpp"
#include <string>
#include <memory>

namespace redis
{
    class redis_db {
    public:
        redis_db(int id) : id_(id) {}

    private:
        std::unordered_map<std::string, std::shared_ptr<redis::redis_object>> dict_{};
        std::unordered_set<std::string> expires_{};
        int id_{-1};
    };
} // namespace redis
