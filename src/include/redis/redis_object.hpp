#pragma once
#include <string>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <memory>
// #include "redis/sds.hpp"
// #include "redis/adlist.hpp"
// #include "redis/dict.hpp"

namespace redis
{
    // a wrapper to store redis type in a dict?
    // struct redis_object
    // {
    // public:
    //     enum class object_type
    //     {
    //         empty,
    //         sds,
    //         list,
    //         dict
    //     };

    //     redis_object() = delete;
    //     redis_object(object_type type) : type_(type) {}
    //     redis_object(const redis_object &) = delete;
    //     redis_object(redis_object &&) = delete;
    //     auto operator=(const redis_object &other) = delete;
    //     auto operator=(redis_object &&) = delete;
    //     virtual ~redis_object() = default;
    //     // virtual redis_object *duplicate() { return nullptr; }

    //     object_type type_{object_type::empty};
    // };

    struct redis_object
    {
    public:
        enum class object_type
        {
            empty,
            sds,
            list,
            set,
            dict
        };

        redis_object() = delete;
        redis_object(object_type type, void *ptr) : type_(type), ptr_(ptr) {}
        redis_object(const redis_object &) = delete;
        redis_object(redis_object &&) = delete;
        auto operator=(const redis_object &other) = delete;
        auto operator=(redis_object &&) = delete;
        ~redis_object()
        {
            if (type_ == object_type::sds)
            {
                delete static_cast<std::string *>(ptr_);
            }
            else if (type_ == object_type::list)
            {
                delete static_cast<std::list<std::string> *>(ptr_);
            }
            else if (type_ == object_type::set)
            {
                delete static_cast<std::unordered_set<std::string> *>(ptr_);
            }
            else if (type_ == object_type::dict)
            {
                delete static_cast<std::unordered_map<std::string, std::shared_ptr<redis_object>> *>(ptr_);
            }
        }

        object_type type_{object_type::empty};
        void *ptr_{nullptr};
    };
} // namespace redis