#pragma once
// #include "redis/sds.hpp"
// #include "redis/adlist.hpp"
// #include "redis/dict.hpp"

namespace redis
{
    // a wrapper to store redis type in a dict?
    struct redis_object
    {
    public:
        enum class object_type
        {
            empty,
            sds,
            list,
            dict
        };

        redis_object() = delete;
        redis_object(object_type type) : type_(type) {}
        redis_object(const redis_object &) = delete;
        redis_object(redis_object &&) = delete;
        auto operator=(const redis_object &other) = delete;
        auto operator=(redis_object &&) = delete;
        virtual ~redis_object() = default;
        // virtual redis_object *duplicate() { return nullptr; }

        object_type type_{object_type::empty};
    };
} // namespace redis