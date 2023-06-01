#pragma once
#include <unordered_map>
#include "redis/redis_object.hpp"

namespace redis
{
    template<typename key, typename value>
    class dict : public redis_object {
    public:
        dict() : redis_object(redis_object::object_type::dict) {}
        dict(const dict&) = default;
        dict(dict&&) = default;
        auto dict(const dict&) -> dict & = default;
        auto dict(dict&&) -> dict & = default;
        ~dict() override = default;

        auto len() const noexcept -> size_t { table_.size(); }

    private:
        std::unordered_map<key, value> table_;
    };
} // namespace redis
