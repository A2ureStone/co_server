#pragma once
#include "redis/redis_object.hpp"
#include <list>

namespace redis
{
    template<typename T>
    class adlist : public redis_object {
    public:
        adlist() : redis_object(0, redis_object::object_type::list) {}
        adlist(const adlist&) = default;
        adlist(adlist&&) = default;
        auto operator(const adlist&) -> adlist & = default;
        auto operator(adlist&&) -> adlist & = default;
        ~adlist() override = default;

        auto length() const noexcept -> size_t {
            return list_.size();
        }

    private:
        std::list<T> list_;
    };
} // namespace redis