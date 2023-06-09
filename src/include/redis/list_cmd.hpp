#pragma once
#include "redis/cmd.hpp"
#include "redis/redis.hpp"
#include "redis/redis_object.hpp"

namespace redis
{
    struct lpush_cmd : public cmd
    {
        lpush_cmd() : cmd("lpush_cmd", 3, 0) {}
        auto execute(redis_client &c) -> void override;
    };

    struct lpop_cmd : public cmd
    {
        lpop_cmd() : cmd("lpop_cmd", 2, 0) {}
        auto execute(redis_client &c) -> void override;
    };

    struct llen_cmd : public cmd
    {
        llen_cmd() : cmd("llen", 2, 0) {}
        auto execute(redis_client &c) -> void override;
    };

} // namespace redis
