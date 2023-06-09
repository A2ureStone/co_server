#pragma once
#include "redis/cmd.hpp"
#include "redis/redis.hpp"
#include "redis/redis_object.hpp"
#include <format>

namespace redis
{
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
        get_cmd() : cmd("get", 2, 0) {}
        auto execute(redis_client &c) -> void override;
        virtual ~get_cmd() override = default;
    };

    struct del_cmd : public cmd
    {
        del_cmd() : cmd("del", -2, 0) {}
        auto execute(redis_client &c) -> void override;
        virtual ~del_cmd() override = default;
    };

    struct exists_cmd : public cmd
    {
        exists_cmd() : cmd("exists", 2, 0) {}
        auto execute(redis_client &c) -> void override;
        virtual ~exists_cmd() override = default;
    };

    struct incr_cmd : public cmd
    {
        incr_cmd() : cmd("incr", 2, 0) {}
        auto execute(redis_client &c) -> void override;
        virtual ~incr_cmd() override = default;
    };

    struct decr_cmd : public cmd
    {
        decr_cmd() : cmd("decr", 2, 0) {}
        auto execute(redis_client &c) -> void override;
        virtual ~decr_cmd() override = default;
    };
} // namespace redis
