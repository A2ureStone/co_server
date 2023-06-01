#include "redis/redis_server.hpp"
#include "redis/anet.hpp"
#include <exception>
#include <string>

namespace redis
{
    redis_server::redis_server()
    {
        // TODO config file support
        // TODO signal process
        // TODO share object create
        svr_sock_ = anet_tcpserver(context_, port_);
        if (!svr_sock_.is_init()) {
            // TODO improve sock init
            throw std::runtime_error("socket init error");
        }
        dbs_.reserve(db_num_);
        for (int id = 0; id < db_num_; ++id) {
            dbs_.emplace_back(id);
        }
        // TODO create time event
    }

    auto redis_server::server_task() -> coro::bg_task<> {
        // std::string ip, int port;
        // int client_sock = anet_accept(context_, svr_sock_, &ip, &port);
        // TODO accept error handle
        co_return;
    }
} // namespace redis