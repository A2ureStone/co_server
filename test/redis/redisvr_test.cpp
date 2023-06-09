#include <gtest/gtest.h>
#include "coro/task.hpp"
#include <iostream>
#include "coro/bg_task.hpp"
#include "coro/net/io_scheduler.hpp"
#include "redis/anet.hpp"
#include "redis/redis_server.hpp"
#include <format>

using coro::bg_task;
using coro::net::io_scheduler;
using redis::redis_server;

TEST(RedisServerTest, OnelineRequest)
{
    redis_server svr;
    auto cli = [&context = svr.context_]() -> bg_task<>
    {
        std::string ip("127.0.0.1");
        int port = 6379;
        auto client_sock = co_await redis::anet_connect(context, ip, port);
        std::string request(90000, '1');
        request.append("\r\n");
        auto write_num = co_await client_sock.write_until(request.data(), request.size());
        std::cout << std::format("write: {}\n", write_num);
        std::string buffer(90002, '0');
        auto read_num = co_await client_sock.recv(buffer.data(), buffer.size());
        std::cout << std::format("recv: {}\n", read_num);
        std::cout << buffer.substr(0, read_num) << std::endl;
        co_return;
    };
    svr.context_.spawn(svr.server_task());
    svr.context_.spawn(cli());
    svr.context_.run();
}