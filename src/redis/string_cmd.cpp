#include "redis/string_cmd.hpp"
#include "redis/sds.hpp"
#include "redis/redis_client.hpp"

namespace redis
{
    auto set_generic_cmd(redis_client &c, bool nx) -> void
    {
        auto &dict = c.db_->dict_;
        if (dict.contains(c.argv_[1]))
        {
            if (nx)
            {
                c.add_reply(shared_obj.czero);
                return;
            }
        }
        dict[c.argv_[1]] = std::make_shared<sds>(c.argv_[2]);
        // why we remove expire?
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
        redis_object *obj = c.db_->lookup_key(c.argv_[1]);
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
            auto sds_ptr = static_cast<sds *>(obj);
            c.add_reply(std::format("${}\r\n{}", sds_ptr->len(), sds_ptr->buf()));
            c.add_reply(shared_obj.crlf);
        }
    }
} // namespace redis
