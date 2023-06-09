#include <gtest/gtest.h>
#include "coro/task.hpp"
#include <iostream>
#include "coro/bg_task.hpp"
#include "coro/net/io_scheduler.hpp"
#include "redis/anet.hpp"
#include "redis/redis_server.hpp"
#include <format>
#include <string>

using coro::bg_task;
using coro::task;
using coro::net::io_scheduler;
using redis::redis_server;

auto check_input_output(io_scheduler &context, const std::string &input, const std::string &output) -> task<bool>
{
    std::string ip("127.0.0.1");
    int port = 6379;
    auto client_sock = co_await redis::anet_connect(context, ip, port);
    std::string buffer(1024, '0');
    auto write_num = co_await client_sock.send(input.data(), input.size());
    EXPECT_EQ(write_num, input.size());
    auto read_num = co_await client_sock.recv(buffer.data(), buffer.size());
    co_return buffer.substr(0, read_num) == output;
}

TEST(SetCmdTest, AddRemTest)
{
    redis_server svr;
    auto cli = [&context = svr.context_]() -> bg_task<>
    {
        EXPECT_EQ(co_await check_input_output(context, "sismember a 1\r\n", ":0\r\n"), true);
        EXPECT_EQ(co_await check_input_output(context, "sadd a 1\r\n", ":1\r\n"), true);
        EXPECT_EQ(co_await check_input_output(context, "sismember a 1\r\n", ":1\r\n"), true);
        EXPECT_EQ(co_await check_input_output(context, "sadd a 1\r\n", ":0\r\n"), true);
        EXPECT_EQ(co_await check_input_output(context, "sadd a 2\r\n", ":1\r\n"), true);
        EXPECT_EQ(co_await check_input_output(context, "srem a 1\r\n", ":1\r\n"), true);
        EXPECT_EQ(co_await check_input_output(context, "srem a 1\r\n", ":0\r\n"), true);
        EXPECT_EQ(co_await check_input_output(context, "srem a 2\r\n", ":1\r\n"), true);
        EXPECT_EQ(co_await check_input_output(context, "sismember a 1\r\n", ":0\r\n"), true);
        co_return;
    };
    svr.context_.spawn(svr.server_task());
    svr.context_.spawn(cli());
    svr.context_.run();
}