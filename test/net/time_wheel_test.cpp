#include <gtest/gtest.h>
#include "coro/net/io_scheduler.hpp"
#include "coro/bg_task.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using coro::bg_task;
using coro::net::io_scheduler;
using namespace std::chrono_literals;

TEST(IoSchedulerTest, TimeoutTest)
{
    io_scheduler context;
    auto func = [&context](std::chrono::seconds time) -> bg_task<>
    {
        // co_await context.wait_for(time);
        EXPECT_EQ(co_await context.wait_for(time), true);
        co_return;
    };
    auto pre = std::chrono::system_clock::now();
    context.spawn(func(1s));
    context.spawn(func(1s));
    context.spawn(func(2s));
    context.spawn(func(2s));
    context.spawn(func(2s));
    context.run();
    auto now = std::chrono::system_clock::now();
    std::cout << "wait for " << std::chrono::duration_cast<std::chrono::milliseconds>(now - pre).count() << "ms\n";
}

TEST(IoSchedulerTest, MaxTimeoutTest)
{
    io_scheduler context;
    auto func = [&context]() -> bg_task<>
    {
        auto pre = std::chrono::system_clock::now();
        EXPECT_EQ(co_await context.wait_for(40s), true);
        auto now = std::chrono::system_clock::now();
        std::cout << "wait for " << std::chrono::duration_cast<std::chrono::milliseconds>(now - pre).count() << "ms\n";
        co_return;
    };
    context.spawn(func());
    context.run();
}

TEST(IoSchedulerTest, ExceedMax)
{
    io_scheduler context;
    auto func = [&context]() -> bg_task<>
    {
        EXPECT_EQ(co_await context.wait_for(41s), false);
        co_return;
    };
    context.spawn(func());
    context.run();
}