#include "redis/set_cmd.hpp"
#include "redis/redis_client.hpp"
#include <unordered_set>

namespace redis
{
    auto sadd_cmd::execute(redis_client &c) -> void
    {
        auto sobj = c.db_->lookup_key_write(c.argv_[1]);
        std::unordered_set<std::string> *set = nullptr;
        if (!sobj)
        {
            set = new std::unordered_set<std::string>();
            c.db_->dict_add(c.argv_[1], std::make_shared<redis_object>(redis_object::object_type::set, set));
        }
        else
        {
            if (sobj->type_ != redis_object::object_type::set)
            {
                c.add_reply(shared_obj.wrongtypeerr);
                return;
            }
            else
            {
                set = static_cast<std::unordered_set<std::string> *>(sobj->ptr_);
            }
        }
        if (auto [it, inserted] = set->emplace(c.argv_[2]); inserted)
        {
            c.add_reply(shared_obj.cone);
        }
        else
        {
            c.add_reply(shared_obj.czero);
        }
    }

    auto srem_cmd::execute(redis_client &c) -> void
    {
        auto sobj = c.db_->lookup_key_write(c.argv_[1]);
        std::unordered_set<std::string> *set = nullptr;
        if (!sobj)
        {
            c.add_reply(shared_obj.czero);
            return;
        }
        else
        {
            if (sobj->type_ != redis_object::object_type::set)
            {
                c.add_reply(shared_obj.wrongtypeerr);
                return;
            }
            else
            {
                set = static_cast<std::unordered_set<std::string> *>(sobj->ptr_);
            }
        }
        if (set->erase(c.argv_[2]) != 0)
        {
            c.add_reply(shared_obj.cone);
        }
        else
        {
            c.add_reply(shared_obj.czero);
        }
    }

    auto sismember_cmd::execute(redis_client &c) -> void
    {
        auto sobj = c.db_->lookup_key_read(c.argv_[1]);
        std::unordered_set<std::string> *set = nullptr;
        if (!sobj)
        {
            c.add_reply(shared_obj.czero);
            return;
        }
        else
        {
            if (sobj->type_ != redis_object::object_type::set)
            {
                c.add_reply(shared_obj.wrongtypeerr);
                return;
            }
            else
            {
                set = static_cast<std::unordered_set<std::string> *>(sobj->ptr_);
            }
        }
        if (set->contains(c.argv_[2]))
        {
            c.add_reply(shared_obj.cone);
        }
        else
        {
            c.add_reply(shared_obj.czero);
        }
    }
} // namespace redis
