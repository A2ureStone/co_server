#pragma once

#include <concepts>
#include <coroutine>
#include <type_traits>
#include <utility>

namespace coro::concepts {
    template <typename type>
    concept awaiter = requires(type t, std::coroutine_handle<> c) {
        {
            t.await_ready()
        } -> std::same_as<bool>;
        requires std::same_as<decltype(t.await_suspend(c)), void> ||
                     std::same_as<decltype(t.await_suspend(c)), bool> ||
                     std::same_as<decltype(t.await_suspend(c)), std::coroutine_handle<>>;
        {
            t.await_resume()
        };
    };

    template <typename type>
    concept awaitable = requires(type t) {
        {
            t.operator co_await()
        } -> awaiter;
    };
}