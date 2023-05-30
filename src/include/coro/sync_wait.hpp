#pragma once
#include <coroutine>
#include "coro/task.hpp"
#include <exception>
#include <mutex>
#include <condition_variable>
#include "coro/concepts/awaitable.hpp"

namespace coro
{
    template <class ReturnType>
    struct sync_wait_task;

    namespace details
    {
        // use to sync coroutine
        class sync_wait_event
        {
        public:
            sync_wait_event() = default;

            sync_wait_event(const sync_wait_event &) = delete;
            sync_wait_event(sync_wait_event &&) = delete;
            auto operator=(const sync_wait_event &) -> sync_wait_event & = delete;
            auto operator=(sync_wait_event &&) -> sync_wait_event & = delete;

            ~sync_wait_event() = default;

            auto set() noexcept -> void;
            auto reset() noexcept -> void;
            auto wait() noexcept -> void;

        private:
            std::mutex mutex_;
            std::condition_variable cv_;
            bool set_{false};
        };

        struct sync_wait_promise_base
        {
            sync_wait_promise_base() noexcept = default;
            ~sync_wait_promise_base() noexcept = default;

            // eager execution
            auto initial_suspend() const noexcept { return std::suspend_always{}; }

            // call at top level, we can let exception propagate
            auto unhandled_exception() noexcept -> void { exception_ptr_ = std::current_exception(); }

        protected:
            sync_wait_event *ev_{nullptr};
            std::exception_ptr exception_ptr_;
        };

        // support reference type
        template <class ReturnType>
        struct sync_wait_promise : public sync_wait_promise_base
        {
            sync_wait_promise() = default;
            ~sync_wait_promise() = default;

            using task_type = sync_wait_task<ReturnType>;
            using coroutine_type = std::coroutine_handle<sync_wait_promise<ReturnType>>;

            auto get_return_object() const noexcept -> task_type
            {
                return task_type{coroutine_type::from_promise(*this)};
            }

            // trick to use return value in the promise to remove copy
            auto yield_value(return_type &&value) noexcept
            {
                res_ = std::addressof(value);
                return final_suspend();
            }

            auto start(sync_wait_event &ev) -> void
            {
                // use for finally suspend
                ev_ = &ev;
                coroutine_type::from_promise(*this).resume();
            }

            auto final_suspend() noexcept
            {
                struct completion_notifer
                {
                    auto await_ready() const noexcept -> bool { return false; }
                    auto await_suspend(coroutine_type coroutine) const noexcept -> void
                    {
                        coroutine.promise().ev_->set();
                    }
                    auto await_resume() const noexcept -> void{};
                };
                return completion_notifer{};
            }

            auto result() -> ReturnType &&
            {
                if (exception_ptr_)
                {
                    std::rethrow_exception(exception_ptr_);
                }
                // optional cast to lvalue base on ReturnType
                return static_cast<ReturnType &&>(*res_);
            }

        private:
            std::remove_reference<ReturnType> *res_;
        };

        template <>
        struct sync_wait_promise<void> : public sync_wait_promise_base
        {
            sync_wait_promise() = default;
            ~sync_wait_promise() = default;

            using task_type = sync_wait_task<void>;
            using coroutine_type = std::coroutine_handle<sync_wait_promise<void>>;

            auto get_return_object() const noexcept -> task_type{
                return task_type{coroutine_type::from_promise(*this)};
            };

            auto start(sync_wait_event &ev) -> void
            {
                // use for finally suspend
                ev_ = &ev;
                coroutine_type::from_promise(*this).resume();
            }

            auto final_suspend() noexcept
            {
                struct completion_notifer
                {
                    auto await_ready() const noexcept -> bool { return false; }
                    auto await_suspend(coroutine_type coroutine) const noexcept -> void
                    {
                        coroutine.promise().ev_->set();
                    }
                    auto await_resume() const noexcept -> void{};
                };
                return completion_notifer{};
            }

            auto result() -> void
            {
                if (exception_ptr_)
                {
                    std::rethrow_exception(exception_ptr_);
                }
            }

        private:
        };

        template <typename ReturnType>
        struct sync_wait_task
        {
            using promise_type = details::sync_wait_promise<ReturnType>;
            using coroutine_handle = std::coroutine_handle<promise_type>

            sync_wait_task(coroutine_handle coroutine) noexcept : coroutine_(coroutine)
            {
            }
            sync_wait_task(sync_wait_task &&t) noexcept : coroutine_(std::exchange(t.coroutine_, nullptr)) {}
            sync_wait_task(const sync_wait_task &) = delete;

            auto operator=(sync_wait_task &&t) -> sync_wait_task
            {
                if (this != std::addressof(t))
                {
                    if (coroutine_)
                    {
                        coroutine_.destroy();
                    }
                    coroutine_ = std::exchange(t.coroutine_, nullptr);
                }
                return *this;
            }
            auto operator=(const sync_wait_task &) -> sync_wait_task = delete;

            ~sync_wait_task()
            {
                if (coroutine_)
                {
                    coroutine_.destroy();
                }
            }

            auto start(sync_wait_task &event) -> void
            {
                coroutine_.promise().start(event);
            }

            auto return_value() -> decltype(auto)
            {
                if constexpr (std::is_same_v<ReturnType, void>)
                {
                    // propagate result
                    coroutine_.promise().result();
                    return;
                }
                else
                {
                    return coroutine_.promise().result();
                }
            }

        private:
            coroutine_handle coroutine_{nullptr};
        };

        template <concepts::awaitable AwaitableType, typename ReturnType>
        static auto make_sync_wait_task(AwaitableType &&a) -> sync_wait_task<ReturnType>
        {
            if constexpr (std::is_same_v<ReturnType, void>)
            {
                co_await std::forward<AwaitableType>(a);
                co_return;
            }
            else
            {
                co_yield co_await std::forward<AwaitableType>(a);
            }
        }
    }

    // why is rvalue reference
    template <concepts::awaitable AwaitableType>
    auto sync_wait(AwaitableType &&a) -> decltype(auto)
    {
        details::sync_wait_event ev;
        auto task = details::make_sync_wait_task(std::forward<awaitable>(a));
        // we may return from here, because task schedule to other threads
        task.start(ev);
        ev.wait();

        return task.return_value();
    }
}