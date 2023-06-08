#pragma once
#include "redis/cmd.hpp"
#include "redis/redis.hpp"
#include "redis/redis_object.hpp"
#include <format>

namespace redis
{
    // auto set_generic_cmd(redis_client &c, bool nx) -> void;
    struct set_cmd : public cmd
    {
        set_cmd() : cmd("set", 3, REDIS_CMD_BULK | REDIS_CMD_DENYOOM) {}
        auto execute(redis_client &c) -> void override;
    };

    struct setnx_cmd : public cmd
    {
        setnx_cmd() : cmd("setnx", 3, REDIS_CMD_BULK | REDIS_CMD_DENYOOM) {}
        auto execute(redis_client &c) -> void override;
        ~setnx_cmd() override = default;
        // {
        //     set_generic_cmd(c, true);
        // }
    };

    struct get_cmd : public cmd
    {
        get_cmd() : cmd("get", 2, REDIS_CMD_INLINE) {}
        auto execute(redis_client &c) -> void override;
        virtual ~get_cmd() override = default;
    };
} // namespace redis
