#pragma once
#include "redis/cmd.hpp"
#include "redis/redis.hpp"
#include "redis/redis_object.hpp"

namespace redis
{
    struct sadd_cmd : public cmd
    {
        sadd_cmd() : cmd("sadd", 3, REDIS_CMD_BULK | REDIS_CMD_DENYOOM) {}
        auto execute(redis_client &c) -> void override;
    };

    struct srem_cmd : public cmd
    {
        srem_cmd() : cmd("srem", 3, 0) {}
        auto execute(redis_client &c) -> void override;
    };

    struct sismember_cmd : public cmd
    {
        sismember_cmd() : cmd("sismember", 3, 0) {}
        auto execute(redis_client &c) -> void override;
    };
} // namespace redis