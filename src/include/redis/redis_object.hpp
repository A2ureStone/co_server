#pragma once

namespace redis
{
    struct redis_object
    {
    public:
        enum class object_type
        {
            sds,
            list,
            dict
        };

        redis_object(int refcount, object_type type) : refcount_(refcount), type_(type) {}
        redis_object(const redis_object&) = default;
        redis_object(redis_object&&) = default;
        auto operator=(const redis_object&) -> redis_object& = default;
        auto operator=(redis_object&&) -> redis_object& = default;
        virtual ~redis_object() = default;

        int refcount_;
        object_type type_;
    };
} // namespace redis