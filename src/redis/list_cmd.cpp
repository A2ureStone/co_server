#include "redis/list_cmd.hpp"
#include "redis/redis_client.hpp"
#include <list>

namespace redis
{
    auto push_generic_cmd(redis_client &c, bool head)
    {
        auto lobj = c.db_->lookup_key_write(c.argv_[1]);
        std::list<std::string> *list = nullptr;
        if (!lobj)
        {
            list = new std::list<std::string>();
            c.db_->dict_add(c.argv_[1], std::make_shared<redis_object>(redis_object::object_type::list, list));
        }
        else
        {
            if (lobj->type_ != redis_object::object_type::list)
            {
                c.add_reply(shared_obj.wrongtypeerr);
                return;
            }
            else
            {
                list = static_cast<std::list<std::string> *>(lobj->ptr_);
            }
        }
        if (head)
        {
            list->push_front(c.argv_[2]);
        }
        else
        {
            list->push_back(c.argv_[2]);
        }
        c.add_reply(shared_obj.ok);
    }

    auto pop_generic_cmd(redis_client &c, bool head)
    {
        auto lobj = c.db_->lookup_key_write(c.argv_[1]);
        if (!lobj)
        {
            c.add_reply(shared_obj.nullbulk);
            return;
        }
        else
        {
            if (lobj->type_ != redis_object::object_type::list)
            {
                c.add_reply(shared_obj.wrongtypeerr);
                return;
            }
            else
            {
                auto list = static_cast<std::list<std::string> *>(lobj->ptr_);
                if (list->empty())
                {
                    c.add_reply(shared_obj.nullbulk);
                    return;
                }
                else
                {
                    std::string val = head ? std::move(list->front()) : std::move(list->back());
                    if (head)
                    {
                        list->pop_front();
                    }
                    else
                    {
                        list->pop_back();
                    }
                    c.add_reply(std::format("${}\r\n{}\r\n", val.size(), val.data()));
                }
            }
        }
    }

    auto lpush_cmd::execute(redis_client &c) -> void
    {
        push_generic_cmd(c, true);
    }

    auto lpop_cmd::execute(redis_client &c) -> void
    {
        pop_generic_cmd(c, true);
    }

    auto llen_cmd::execute(redis_client &c) -> void
    {
        auto lobj = c.db_->lookup_key_read(c.argv_[1]);
        if (!lobj)
        {
            c.add_reply(shared_obj.czero);
        }
        else
        {
            if (lobj->type_ != redis_object::object_type::list)
            {
                c.add_reply(shared_obj.wrongtypeerr);
            }
            else
            {
                auto list = static_cast<std::list<std::string> *>(lobj->ptr_);
                c.add_reply(std::format(":{}\r\n", list->size()));
            }
        }
    }
} // namespace redis