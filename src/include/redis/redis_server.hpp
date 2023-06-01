#pragma once
#include "redis/redis_db.hpp"
#include "redis/redis_client.hpp"
#include "coro/net/socket.hpp"
#include "coro/net/io_scheduler.hpp"
#include "coro/bg_task.hpp"
#include "redis/redis.hpp"
#include <array>
#include <vector>
#include <list>
#include <unordered_set>
#include <string>

namespace redis
{
    // forward declare
    class redis_client;

    class redis_server
    {
    public:
        redis_server();

        auto server_task() -> coro::bg_task<>;

    public:
        int port_{REDIS_SERVERPORT};
        int db_num_{REDIS_DEFAULT_DBNUM};
        size_t client_max_querybuf_len_{REDIS_MAX_QUERYBUF_LEN};

        coro::net::io_scheduler context_{};
        coro::net::socket svr_sock_{};
        std::list<redis_client> clients{};
        std::list<redis_client> slaves_{};
        std::list<redis_client> monitors_{};
        std::list<redis_client> objfreelist_{};
        std::vector<redis_db> dbs_{};
        // std::unordered_set<std::string> shardingpool_;
    };
} // namespace redis
