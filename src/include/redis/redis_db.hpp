#pragma once
#include <unordered_map>
#include <unordered_set>
#include "redis/redis_object.hpp"
#include <string>
#include <memory>
#include <chrono>

namespace redis
{
    using time = std::chrono::system_clock::time_point;
    class redis_db
    {
    public:
        redis_db(int id) : id_(id) {}

        auto dict_add(const std::string &key, const std::shared_ptr<redis_object> &ob);

        auto lookup_key(const std::string &key) -> redis_object *;

        auto delete_key(const std::string &key) -> bool;

        auto set_expire(const std::string &key, const time &tp) -> bool;

        auto get_expire(const std::string &key) -> std::optional<time>;

        auto remove_expire(const std::string &key) -> bool;

        // we use pointer to call virtual destructor
        std::unordered_map<std::string, std::shared_ptr<redis_object>> dict_{};
        std::unordered_map<std::string, std::chrono::system_clock::time_point> expires_{};
        int id_{-1};
    };
} // namespace redis
