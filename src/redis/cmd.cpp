#include "redis/cmd.hpp"
#include "redis/string_cmd.hpp"
#include <iostream>

namespace redis
{
    command_table cmd_table;

    command_table::command_table()
    {
        table_["set"] = std::make_unique<set_cmd>();
        table_["setnx"] = std::make_unique<setnx_cmd>();
        table_["get"] = std::make_unique<get_cmd>();
    }

    auto command_table::lookup_cmd(const std::string &name) -> cmd *
    {
        // std::cout << "lookup_cmd: " << name << std::endl;
        return table_.contains(name) ? table_[name].get() : nullptr;
    }

} // namespace reds
