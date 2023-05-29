#include <gtest/gtest.h>
#include "coro/bg_task.hpp"
#include <iostream>

using coro::bg_task;

struct foo {
    foo() {}
    foo(const foo &) { std::cout << "cp ctr\n";  }
    foo(foo &&) { std::cout << "mv ctr\n";  }
    foo &operator=(const foo &) { std::cout << "cp ass\n"; return *this; }
    foo &operator=(foo &&) { std::cout << "mv ass\n"; return *this; }
    ~foo() { std::cout << "dtr\n"; }
    void print() {}
};

TEST(BgTaskTest, LazyExecution)
{
    int x = 1;
    auto func = [&]() -> bg_task<>
    {
        x = 2;
        co_return;
    };
    auto t = func();
    ASSERT_EQ(x, 1);
    t.resume();
    ASSERT_EQ(x, 2);
}

TEST(BgTaskTest, AutoDestroy)
{
    int x = 1;
    auto func = [&](foo f) -> bg_task<>
    {
        x = 2;
        co_return;
    };
    auto t = func(foo());
    ASSERT_EQ(x, 1);
    std::cout << "before dtr\n";
    t.resume();
    std::cout << "after dtr\n";
    ASSERT_EQ(x, 2);
}