#include "redis/redis_db.hpp"
#include <optional>

namespace redis
{
    auto redis_db::dict_add(const std::string &key, const std::shared_ptr<redis_object> &obj) -> void
    {
        dict_[key] = obj;
    }

    auto redis_db::lookup_key(const std::string &key) -> redis_object *
    {
        // TODO expired impl
        return dict_.contains(key) ? dict_[key].get() : nullptr;
    }

    auto redis_db::lookup_key_read(const std::string &key) -> redis_object *
    {
        expire_if_needed(key);
        return lookup_key(key);
    }

    auto redis_db::lookup_key_write(const std::string &key) -> redis_object *
    {
        // TODO dirty with deleteIfVolatile
        expire_if_needed(key);
        return lookup_key(key);
    }

    auto redis_db::delete_key(const std::string &key) -> bool
    {
        // delete expire?
        return dict_.erase(key) != 0;
    }

    auto redis_db::set_expire(const std::string &key, const time &tp) -> bool
    {
        if (expires_.contains(key))
        {
            return false;
        }
        expires_[key] = tp;
        return true;
    }

    auto redis_db::get_expire(const std::string &key) -> std::optional<time>
    {
        return expires_.contains(key) ? std::make_optional<time>(expires_[key]) : std::nullopt;
    }

    auto redis_db::remove_expire(const std::string &key) -> bool
    {
        return expires_.erase(key) != 0;
    }

    auto redis_db::expire_if_needed(const std::string &key) -> bool
    {
        if (!expires_.contains(key))
        {
            return false;
        }
        auto when = expires_[key];
        if (when <= std::chrono::system_clock::now())
        {
            return false;
        }
        // we need to delete the key
        expires_.erase(key);
        return dict_.erase(key) != 0;
    }

    // auto redis_db::delete_if_volatile(const std::string &key) -> bool
    // {
    //     if (!expires_.contains(key))
    //     {
    //         return false;
    //     }
    //     auto when = expires_[key];
    //     if (when <= std::chrono::system_clock::now())
    //     {
    //         return false;
    //     }
    //     // we need to delete the key
    //     expires_.erase(key);
    //     return dict_.erase(key) != 0;
    // }
}
