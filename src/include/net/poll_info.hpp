#pragma once
#include <coroutine>
#include <sys/uio.h>

namespace coro
{
    enum class poll_op
    {
        noop,
        connect,
        accept,
        read,
        write,
        timeout
    };

    enum class poll_staus
    {
        event,
        timeout,
        error
    };

    auto stat_code_string(int status_code) -> const char *;

    namespace net
    {
        struct poll_info
        {
            poll_info() = default;
            ~poll_info() = default;

            poll_info(const poll_info &) = delete;
            poll_info(poll_info &&) = delete;
            auto operator=(const poll_info &) -> poll_info & = delete;
            auto operator=(poll_info &&) -> poll_info & = delete;

            struct poll_awaiter
            {
                poll_awaiter(poll_info &info) noexcept : info_(info){};

                auto await_ready() const noexcept -> bool { return false; }

                auto await_suspend(std::coroutine_handle<> awaiting_coro) const noexcept -> void
                {
                    info_.awaiting_coro_ = awaiting_coro;
                    // TODO multi-thread sync
                }

                auto await_resume(){};

            private:
                poll_info &info_;
            };

            auto operator co_await() noexcept -> poll_awaiter { return poll_awaiter{*this}; }

            // int fd_{-1};
            std::coroutine_handle<> awaiting_coro_{nullptr};
            poll_op operation_{poll_op::noop};
            static constexpr int iovec_count{1};
            struct iovec iov_;
            int res_{-1};
        };
    } // namespace net

} // namespace coro
