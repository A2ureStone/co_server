#include <gtest/gtest.h>
#include "redis/anet.hpp"
#include "coro/net/io_scheduler.hpp"
#include "coro/bg_task.hpp"
#include "coro/net/socket.hpp"
#include <liburing.h>
#include <format>

using coro::bg_task;
using coro::net::io_scheduler;
using namespace redis;
TEST(AnetTest, TestSocketWrapper)
{

    io_scheduler context;
    auto server = [&context]() -> bg_task<>
    {
        auto sock = anet_tcpserver(context, 8103);
        int client_fd = co_await anet_accept(context, sock, nullptr, nullptr);
        if (client_fd < 0)
        {
            EXPECT_TRUE(false);
            co_return;
        }
        char buf[4096];
        buf[4095] = '\0';
        int read_num = 0;
        auto client = coro::net::socket(client_fd, &context);
        if ((read_num = co_await client.recv(buf, 1024)) <= 0)
        {
            EXPECT_TRUE(false);
        }
        co_return;
    };
    auto client = [&context]() -> bg_task<>
    {
        auto sock = co_await anet_connect(context, "127.0.0.1", 8103);
        if (!sock.is_init())
        {
            EXPECT_TRUE(false);
            co_return;
        }
        std::string buf(512, 'a');
        int send_num = co_await sock.send(buf.c_str(), 512);
        if (send_num != 512)
        {
            EXPECT_TRUE(false);
            co_return;
        }
        co_return;
    };
    context.spawn(server());
    context.spawn(client());
    context.run();
}

TEST(AnetTest, MakeTcpServer)
{
    io_scheduler context;
    auto server = [&context]() -> bg_task<>
    {
        auto sock = anet_tcpserver(context, 8103);
        int client_fd = co_await anet_accept(context, sock, nullptr, nullptr);
        EXPECT_TRUE(client_fd > 0);
        if (client_fd < 0)
        {
            co_return;
        }
        char buf[4096];
        buf[4095] = '\0';
        int read_num = 0;
        auto client = coro::net::socket(client_fd, &context);
        if ((read_num = co_await client.read_until(buf, 1024)) != 1024)
        {
            EXPECT_TRUE(false);
        }
        co_return;
    };
    auto client = [&context]() -> bg_task<>
    {
        auto sock = co_await anet_connect(context, "127.0.0.1", 8103);
        if (!sock.is_init())
        {
            EXPECT_TRUE(false);
            co_return;
        }
        std::string buf(512, 'a');
        int send_num = co_await sock.write_until(buf.c_str(), 512);
        if (send_num != 512)
        {
            EXPECT_TRUE(false);
            co_return;
        }
        send_num = co_await sock.write_until(buf.c_str(), 512);
        if (send_num != 512)
        {
            EXPECT_TRUE(false);
            co_return;
        }
        co_return;
    };
    context.spawn(server());
    context.spawn(client());
    context.run();
}

TEST(AnetTest, CancelTest)
{
    io_scheduler context;
    auto server = [&context]() -> bg_task<>
    {
        auto sock = anet_tcpserver(context, 8103);
        int client_fd = co_await anet_accept(context, sock, nullptr, nullptr);
        EXPECT_EQ(-client_fd, ECANCELED);
        co_return;
    };
    auto client = [&context]() -> bg_task<>
    {
        co_await context.notify_on_fd(4);
        co_return;
    };
    context.spawn(server());
    context.spawn(client());
    context.run();
}
