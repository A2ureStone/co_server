#pragma once
#include "redis/redis_object.hpp"
#include <string>
#include <cstring>

namespace redis
{
    class sds : public redis_object {
        sds() : redis_object(0, redis_object::object_type::sds) {};
        // no exception handle, and nullptr check(both for exception) for stl
        sds(const char* init) : redis_object(0, redis_object::object_type::sds), buf_(init) {}
        sds(const sds&) = default;
        sds(sds&&) = default;
        auto operator=(const sds&) -> sds& = default;
        auto operator=(sds&&) -> sds& = default;
        ~sds() override = default;

        auto len() const noexcept -> size_t { return buf_.size(); }
        auto avail() const noexcept -> size_t { return buf_.capacity() - buf_.size(); }
        // reserver room? 
        // exception handle?
        auto concat(const char *t) -> void {
            if (t == nullptr) {
                return;
            }
            buf_.append(t);
        }
        auto cpy(char* t) -> void {
            size_t len = strlen(t);
            buf_.reserve(len);
            buf_.copy(t, len);
        }
        // sprintf with auto start

    private:
        std::string buf_;
    };
} // namespace redis