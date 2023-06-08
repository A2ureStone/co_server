#include "redis/redis_db.hpp"
#include <optional>

namespace redis
{
    auto redis_db::dict_add(const std::string &key, const std::shared_ptr<redis_object> &obj)
    {
    }

    auto redis_db::lookup_key(const std::string &key) -> redis_object *
    {
        // TODO expired impl
        return dict_.contains(key) ? dict_[key].get() : nullptr;
    }

    auto redis_db::delete_key(const std::string &key) -> bool
    {
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
} // namespace redis
