#pragma once
#include "coro/net/socket.hpp"
#include "redis/redis_db.hpp"
#include "redis/redis_object.hpp"
#include "coro/bg_task.hpp"
#include "coro/task.hpp"
#include "redis/redis.hpp"
#include "redis/redis_server.hpp"
#include <list>
#include <memory>

namespace redis
{
    enum class req_type
    {
        empty,
        oneline,
        multibulk
    };

    // forward declare
    class redis_server;

    class redis_client
    {
    public:
        redis_client(int fd, coro::net::io_scheduler *context_ptr);

        auto client_task() -> coro::bg_task<>;

        auto process_intput_buffer() -> coro::task<>;

        auto process_oneline_buffer() -> op_status;

        auto process_multibulk_buffer() -> op_status;

    private:
        coro::net::socket cli_sock_{};
        redis::redis_db *db_{nullptr};
        int dict_id_{-1};
        std::string query_buf_;
        size_t query_buf_peek_{0};
        req_type reqtype_{req_type::empty};
        int argc_{0};
        // redis_object **argv_{nullptr};
        std::vector<std::string> argv_;
        int multi_bulklen_{0};
        int bulklen_{-1};
        int sentlen_{0};
        std::list<std::shared_ptr<redis_object>> reply_{};
        redis_server *svr_ptr_;
    };
} // namespace redis
