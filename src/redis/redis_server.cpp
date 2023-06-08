#include "redis/redis_server.hpp"
#include "redis/anet.hpp"
#include <exception>
#include <string>

namespace redis
{
    sharedObjectsStruct shared_obj;

    redis_server::redis_server()
    {
        // TODO config file support
        // TODO signal process
        // TODO share object create
        svr_sock_ = anet_tcpserver(context_, port_);
        if (!svr_sock_.is_init())
        {
            // TODO improve sock init
            throw std::runtime_error("socket init error");
        }
        dbs_.reserve(db_num_);
        for (int id = 0; id < db_num_; ++id)
        {
            dbs_.emplace_back(id);
        }
        // TODO create time event
    }

    auto redis_server::server_task() -> coro::bg_task<>
    {
        // std::string ip, int port;
        // int client_sock = anet_accept(context_, svr_sock_, &ip, &port);
        while (true)
        {
            int client_sock = co_await anet_accept(context_, svr_sock_, nullptr, nullptr);
            // TODO accept error handle
            context_.spawn(redis_client::read_coroutine(std::make_shared<redis_client>(client_sock, &context_, this)));
        }
        co_return;
    }

    auto redis_server::run() -> void
    {
        context_.spawn(server_task());
        context_.run();
    }
} // namespace redis