#include <iostream>
#include "coro/task.hpp"
#include "coro/concepts/awaitable.hpp"

int main() {
    // int x = 1;
    // auto t = [&x]() -> coro::task<>
    // {
    //     std::cout << "before x\n";
    //     x = 64;
    //     std::cout << x << std::endl;
    //     std::cout << "Hello Wolrd\n";
    //     co_return;
    // }();
    // std::cout << "lazy back\n";
    // // t.resume();
    // t.resume();
    std::cout << "enter resume\n";
    return 0;
}