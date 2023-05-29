#include <gtest/gtest.h>
#include "coro/task.hpp"
#include <iostream>

using coro::task;

struct foo {
    foo() {}
    foo(const foo &) { std::cout << "cp ctr\n";  }
    foo(foo &&) { std::cout << "mv ctr\n";  }
    foo &operator=(const foo &) { std::cout << "cp ass\n"; return *this; }
    foo &operator=(foo &&) { std::cout << "mv ass\n"; return *this; }
    void print() {}
};

TEST(TaskTest, LazyExecution)
{
    int x = 1;
    auto func = [&]() -> task<>
    {
        x = 2;
        co_return;
    };
    auto t = func();
    ASSERT_TRUE(!t.is_ready());
    ASSERT_FALSE(t.resume());
    ASSERT_EQ(x, 2);
    ASSERT_TRUE(t.is_ready());
}

TEST(TaskTest, GetResultInPromise)
{
    auto func = []() -> task<int>
    {
        co_return 2;
    };
    auto t = func();
    ASSERT_TRUE(!t.is_ready());
    ASSERT_FALSE(t.resume());
    ASSERT_EQ(t.promise().result(), 2);
    ASSERT_TRUE(t.is_ready());
}

TEST(TaskTest, CoAwaitTest)
{
    auto func = []() -> task<void>
    {
        auto inner = []() -> task<int>
        {
            co_return 2;
        };
        auto res = co_await inner();
        EXPECT_EQ(res, 2);
        co_return;
    };
    auto t = func();
    ASSERT_TRUE(!t.is_ready());
    ASSERT_FALSE(t.resume());
    ASSERT_TRUE(t.is_ready());
}

TEST(TaskTest, MoveTest)
{
    auto func = []() -> task<void>
    {
        auto inner = []() -> task<foo>
        {
            co_return foo{};
        };
        auto res = co_await inner();
        res.print();
        co_return;
    };
    auto t = func();
    ASSERT_TRUE(!t.is_ready());
    ASSERT_FALSE(t.resume());
    ASSERT_TRUE(t.is_ready());
}

TEST(TaskTest, ThrowExceptionInVoid)
{
    auto func = []() -> task<void>
    {
        auto inner = []() -> task<void>
        {
            throw std::exception{};
            EXPECT_TRUE(false);
            co_return;
        };
        try
        {
            co_await inner();
            EXPECT_TRUE(false);
        }
        catch (...)
        {
        }
        co_return;
    };
    auto t = func();
    ASSERT_TRUE(!t.is_ready());
    ASSERT_FALSE(t.resume());
    ASSERT_TRUE(t.is_ready());
}

TEST(TaskTest, ThrowExceptionInNonVoid)
{
    auto func = []() -> task<void>
    {
        auto inner = []() -> task<int>
        {
            throw std::exception{};
            EXPECT_TRUE(false);
            co_return 2;
        };
        try
        {
            co_await inner();
            EXPECT_TRUE(false);
        }
        catch (...)
        {
        }
        co_return;
    };
    auto t = func();
    ASSERT_TRUE(!t.is_ready());
    ASSERT_FALSE(t.resume());
    ASSERT_TRUE(t.is_ready());
}

TEST(TaskTest, SuspendsMutiple) {
    auto t = []() -> task<void>
    {
        co_await std::suspend_always{};
        co_await std::suspend_always{};
        co_await std::suspend_always{};
    }();
    ASSERT_TRUE(!t.is_ready());
    ASSERT_TRUE(t.resume());

    ASSERT_TRUE(!t.is_ready());
    ASSERT_TRUE(t.resume());

    ASSERT_TRUE(!t.is_ready());
    ASSERT_TRUE(t.resume());

    ASSERT_TRUE(!t.is_ready());
    ASSERT_FALSE(t.resume());

    ASSERT_TRUE(t.is_ready());
}