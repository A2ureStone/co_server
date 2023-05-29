#include <gtest/gtest.h>
#include "net/io_scheduler.hpp"
#include "coro/bg_task.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using coro::bg_task;
using coro::net::io_scheduler;

TEST(IoSchedulerTest, ReadTestLazy)
{
    io_scheduler context;
    auto func = [&context]() -> bg_task<>
    {
        int fd = open("../../CMakeLists.txt", O_RDONLY);
        if (fd < 0)
        {
            std::cerr << "open file error\n";
        }
        std::string buf(1001, '\0');
        int res;
        std::cout << "yield\n";
        if ((res = co_await context.recv(fd, buf.data(), buf.size() - 1)) > 0)
        {
            buf[res] = '\0';
            std::cout << buf << std::endl;
        }
        close(fd);
    };
    context.spawn(func());
    context.spawn(func());
    context.run();
}

TEST(IoSchedulerTest, SpawnSpawn)
{
    io_scheduler context;
    auto func = [&context]() -> bg_task<>
    {
        int fd = open("../../CMakeLists.txt", O_RDONLY);
        if (fd < 0)
        {
            std::cerr << "open file error\n";
        }
        std::string buf(1001, '\0');
        int res;
        context.spawn([]() -> bg_task<> {
            std::cout << "forking coro\n";
            co_return; 
        }());
        std::cout << "yield\n";
        if ((res = co_await context.recv(fd, buf.data(), buf.size() - 1)) > 0)
        {
            buf[res] = '\0';
            std::cout << buf << std::endl;
        }
        close(fd);
        co_return;
    };
    context.spawn(func());
    context.run();
}

TEST(IoSchedulerTest, EmptyTask)
{
    io_scheduler context;
    context.run();
}

TEST(IoSchedulerTest, ServerClient)
{
    io_scheduler context;
    auto server = [&context]() -> bg_task<>
    {
        int sock = socket(PF_INET, SOCK_STREAM, 0);
        int port = 8084;
        if (sock == -1)
        {
            std::cerr << "sock init error\n";
            co_return;
        }
        struct sockaddr_in svr_addr;
        svr_addr.sin_family = AF_INET;
        svr_addr.sin_port = htons(port);
        svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(sock, reinterpret_cast<const sockaddr*>(&svr_addr), sizeof(svr_addr)) < 0)
        {
            std::cerr << "bind error\n";
            co_return;
        }
        if (listen(sock, 10) < 0) {
            std::cerr << "listen error\n";
            co_return;
        }
        int client_fd = co_await context.accept(sock, nullptr, nullptr);
        std::cout << "server accept " << client_fd << std::endl;
        if (client_fd < 0) {
            std::cerr << "accept fail\n";
            co_return;
        }
        char buf[4096];
        buf[4095] = '\0';
        int read_num = 0;
        if ((read_num = co_await context.recv(client_fd, buf, 4096)) > 0)
        {
            co_await context.send(client_fd, buf, read_num);
        }
        std::cout << "server recv " << buf << std::endl;
        close(client_fd);
        close(sock);
        co_return;
    };
    auto client = [&context]() -> bg_task<>
    {
        int sock = socket(PF_INET, SOCK_STREAM, 0);
        int port = 8084;
        if (sock == -1)
        {
            std::cerr << "sock init error\n";
            co_return;
        }
        struct sockaddr_in svr_addr;
        svr_addr.sin_family = AF_INET;
        svr_addr.sin_port = htons(port);
        svr_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (co_await context.connect(sock, reinterpret_cast<sockaddr*>(&svr_addr), sizeof(svr_addr)) < 0)
        {
            std::cerr << "connect sock error\n";
            co_return;
        }
        char buf[4096] = "Hello World";
        std::cout << "client send " << buf << std::endl;
        int read_num = 0;
        if ((read_num = co_await context.send(sock, buf, 12)) != 12)
        {
            std::cerr << "send error\n";
            co_return;
        }
        close(sock);
        co_return;
    };
    context.spawn(server());
    context.spawn(client());
    context.run();
}