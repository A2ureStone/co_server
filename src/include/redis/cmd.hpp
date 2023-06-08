#pragma once
#include <string>
#include <memory>
#include <unordered_map>
// #include "redis/redis_client.hpp"

namespace redis
{
    class redis_client;

    struct cmd
    {
    public:
        cmd(std::string name, int arity, int flags) : name_(std::move(name)), arity_(arity), flags_(flags)
        {
        }
        virtual ~cmd() = default;
        virtual void execute(redis_client &c) = 0;

        std::string name_{};
        int arity_{};
        int flags_{};
    };

    struct command_table
    {
    public:
        using cmd_table_t = std::unordered_map<std::string, std::unique_ptr<cmd>>;
        command_table();

        auto lookup_cmd(const std::string &name) -> cmd *;

        cmd_table_t table_{};
    };

    extern command_table cmd_table;

} // namespace redis
