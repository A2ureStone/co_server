#include "redis/string_cmd.hpp"
#include "redis/redis_client.hpp"
#include <string>

namespace redis
{
    auto set_generic_cmd(redis_client &c, bool nx) -> void
    {
        if (c.db_->lookup_key(c.argv_[1]) != nullptr)
        {
            if (nx)
            {
                c.add_reply(shared_obj.czero);
                return;
            }
        }
        c.db_->dict_add(c.argv_[1], std::make_shared<redis_object>(redis_object::object_type::sds, new std::string(c.argv_[2])));
        c.db_->remove_expire(c.argv_[1]);
        c.add_reply(nx ? shared_obj.cone : shared_obj.ok);
    }

    auto set_cmd::execute(redis_client &c) -> void
    {
        set_generic_cmd(c, false);
    }

    auto setnx_cmd::execute(redis_client &c) -> void
    {
        set_generic_cmd(c, true);
    }

    auto get_cmd::execute(redis_client &c) -> void
    {
        redis_object *obj = c.db_->lookup_key_read(c.argv_[1]);
        if (obj == nullptr)
        {
            c.add_reply(shared_obj.nullbulk);
        }
        else
        {
            if (obj->type_ != redis_object::object_type::sds)
            {
                c.add_reply(shared_obj.wrongtypeerr);
            }
            auto ptr = static_cast<std::string *>(obj->ptr_);
            c.add_reply(std::format("${}\r\n{}\r\n", ptr->size(), ptr->data()));
        }
    }

    auto del_cmd::execute(redis_client &c) -> void
    {
        int deleted = 0;
        for (auto it = c.argv_.begin() + 1; it != c.argv_.end(); ++it)
        {
            // TODO expires check??
            if (c.db_->delete_key(*it))
            {
                ++deleted;
            }
        }
        switch (deleted)
        {
        case 0:
            c.add_reply(shared_obj.czero);
            break;
        case 1:
            c.add_reply(shared_obj.cone);
            break;
        default:
            c.add_reply(std::format(":{}\r\n", deleted));
        }
    }

    auto incr_decr_cmd(redis_client &c, long long incr) -> void
    {
        redis_object *obj = c.db_->lookup_key_write(c.argv_[1]);
        long long value;
        if (obj == nullptr)
        {
            value = 0;
        }
        else
        {
            if (obj->type_ != redis_object::object_type::sds)
            {
                value = 0;
            }
            else
            {
                auto ptr = static_cast<std::string *>(obj->ptr_);
                value = std::stoll(ptr->data());
            }
        }
        value += incr;

        c.db_->dict_add(c.argv_[1], std::make_shared<redis_object>(redis_object::object_type::sds, new std::string(std::to_string(value))));
        c.db_->remove_expire(c.argv_[1]);

        // TODO dirty impl
        c.add_reply(std::format(":{}\r\n", value));
    }

    auto exists_cmd::execute(redis_client &c) -> void
    {
        c.add_reply(c.db_->lookup_key_read(c.argv_[1]) ? shared_obj.cone : shared_obj.czero);
    }

    auto incr_cmd::execute(redis_client &c) -> void
    {
        incr_decr_cmd(c, 1);
    }

    auto decr_cmd::execute(redis_client &c) -> void
    {
        incr_decr_cmd(c, -1);
    }
} // namespace redis
