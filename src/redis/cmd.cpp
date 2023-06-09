#include "redis/cmd.hpp"
#include "redis/string_cmd.hpp"
#include "redis/list_cmd.hpp"
#include "redis/set_cmd.hpp"
#include <iostream>

namespace redis
{
    command_table cmd_table;

    command_table::command_table()
    {
        // string commands
        table_["set"] = std::make_unique<set_cmd>();
        table_["setnx"] = std::make_unique<setnx_cmd>();
        table_["get"] = std::make_unique<get_cmd>();
        table_["del"] = std::make_unique<del_cmd>();
        table_["exists"] = std::make_unique<exists_cmd>();
        table_["incr"] = std::make_unique<incr_cmd>();
        table_["decr"] = std::make_unique<decr_cmd>();
        // list commands
        table_["lpush"] = std::make_unique<lpush_cmd>();
        table_["lpop"] = std::make_unique<lpop_cmd>();
        table_["llen"] = std::make_unique<llen_cmd>();
        // set commands
        table_["sadd"] = std::make_unique<sadd_cmd>();
        table_["srem"] = std::make_unique<srem_cmd>();
        table_["sismember"] = std::make_unique<sismember_cmd>();
    }

    auto command_table::lookup_cmd(const std::string &name) -> cmd *
    {
        // std::cout << "lookup_cmd: " << name << std::endl;
        return table_.contains(name) ? table_[name].get() : nullptr;
    }

} // namespace reds
