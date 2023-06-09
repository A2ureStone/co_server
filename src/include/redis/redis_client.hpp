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
#include <chrono>

namespace redis
{
    // forward declare
    struct cmd;

    enum class req_type
    {
        empty,
        oneline,
        multibulk
    };

    // forward declare
    class redis_server;

    class redis_client : public std::enable_shared_from_this<redis_client>
    {
    public:
        redis_client(int fd, coro::net::io_scheduler *context_ptr, redis_server *svr_ptr);

        ~redis_client();

        /* main loop for client */
        auto read_request_task() -> coro::task<void>;

        // static auto read_coroutine(int fd, coro::net::io_scheduler *context_ptr, redis_server *svr_ptr) -> coro::bg_task<>;
        static auto read_coroutine(std::shared_ptr<redis_client> cli) -> coro::bg_task<>;

        static auto write_coroutine(std::shared_ptr<redis_client> cli) -> coro::bg_task<>;

        auto process_intput_buffer() -> void;

        auto process_oneline_buffer() -> op_status;

        /* parse oneline starting at buffer, check for max_inline limit, return the first pos of '\r\n'  */
        auto try_parse_oneline(size_t &newline) -> op_status;

        auto process_multibulk_buffer() -> op_status;

        /* coroutine to send reply to client */
        auto send_reply_task() -> coro::task<>;

        /* add message to send buffer */
        auto add_reply(std::string message) -> void;

        /* reset client state to be able to process next command */
        auto reset_client() -> void;

        /* Helper function. Trims query buffer to make the function that processes
         * multi bulk requests idempotent. */
        auto set_protocol_error() -> void;

        /* do command */
        auto process_cmd() -> bool;

        auto select_db(int id) -> bool;

    private:
        coro::net::socket cli_sock_{};
        coro::net::io_scheduler *context_ptr_{nullptr};
        bool running_{false};
        int dict_id_{-1};
        std::string query_buf_{};
        size_t query_buf_peek_{0};
        req_type reqtype_{req_type::empty};
        int multi_bulklen_{0};
        int bulklen_{-1};
        std::list<std::string> reply_{};
        size_t totwritten_{0};
        cmd *cmd_{nullptr};

        std::chrono::system_clock::time_point lastinteraction_{}; /* time of the last interaction, used for timeout */
        int flags_;                                               /* REDIS_CLOSE | REDIS_SLAVE | REDIS_MONITOR */
        int slaveseldb_;                                          /* slave selected db, if this client is a slave */
        int authenticated_;                                       /* when requirepass is non-NULL */
        int replstate_;                                           /* replication state if this is a slave */
        int repldbfd_;                                            /* replication DB file descriptor */
        long repldboff_;                                          /* replication DB file offset */
        off_t repldbsize_;                                        /* replication DB file size */
    public:
        redis_server *svr_ptr_{nullptr};
        redis::redis_db *db_{nullptr};
        std::vector<std::string> argv_{};
    };
} // namespace redis
