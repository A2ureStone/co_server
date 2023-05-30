#include <gtest/gtest.h>
#include "redis/anet.hpp"
#include "coro/net/io_scheduler.hpp"
#include "coro/bg_task.hpp"
#include "coro/net/socket.hpp"
#include <format>

using coro::bg_task;
using coro::net::io_scheduler;
using namespace redis;
TEST(AnetTest, MakeTcpServer)
{
    io_scheduler context;
    auto server = [&context]() -> bg_task<>
    {
        auto sock = anet_tcpserver(context, 8103);
        int client_fd = co_await anet_accept(context, sock, nullptr, nullptr);
        std::cout << std::format("server: accept: {}\n", client_fd);
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
            std::cerr << std::format("server: read {} not 1024\n", read_num);
            if (read_num < 0) {
                std::cerr << strerror(-read_num) << std::endl;
            }
        }
        std::cout << "server: wake up from read_until\n";
        co_return;
    };
    auto client = [&context]() -> bg_task<>
    {
        auto sock = co_await anet_connect(context, "127.0.0.1", 8103);
        if (!sock.is_init())
        {
            co_return;
        }
        std::string buf(512, 'a');
        int send_num = co_await sock.write_until(buf.c_str(), 512);
        if (send_num != 512) {
            std::cerr << std::format("client: send num {} error\n", send_num);
            co_return;
        }
        std::cout << "client: send 512 bytes\n";
        send_num = co_await sock.write_until(buf.c_str(), 512);
        if (send_num != 512) {
            std::cerr << std::format("send num {} error\n", send_num);
            co_return;
        }
        std::cout << "client: send 512 bytes\n";
        co_return;
    };
    context.spawn(server());
    context.spawn(client());
    context.run();
}