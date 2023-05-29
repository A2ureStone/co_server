#pragma once
#include <coroutine>
#include <exception>
#include <utility>

namespace coro
{
    template <typename ReturnType = void>
    class bg_task;

    namespace detail
    {
        // destroy coroutine frame automatically
        struct bg_promise
        {
            using coroutine_handle = std::coroutine_handle<bg_promise>;
            using task_type = bg_task<void>;
            struct final_awaitable
            {
                auto await_ready() const noexcept -> bool { return true; }

                auto await_suspend(std::coroutine_handle<>) const noexcept -> void {}

                auto await_resume() const noexcept -> void {}
            };

            bg_promise() noexcept = default;
            ~bg_promise() noexcept = default;

            auto get_return_object() noexcept -> task_type;

            // lazy execution
            auto initial_suspend() const noexcept
            {
                // std::cout << "start suspends\n";
                return std::suspend_always{};
            }

            auto final_suspend() const noexcept { return final_awaitable{}; }

            auto unhandled_exception() noexcept -> void { exception_ptr_ = std::current_exception(); }

            auto return_void() const noexcept -> void {}

        protected:
            std::exception_ptr exception_ptr_{};
        };
    } // namespace detail

    template <>
    class bg_task<void>
    {
    public:
        using promise_type = detail::bg_promise;
        using coroutine_handle = std::coroutine_handle<promise_type>;

        bg_task(coroutine_handle coroutine) noexcept : coroutine_(coroutine) {}
        ~bg_task() = default;

        // delete copy and move operation
		bg_task() = delete;
		bg_task(const bg_task &t) = delete;
		auto operator=(const bg_task &t) -> bg_task & = delete;
		bg_task(bg_task &&t) noexcept : coroutine_(std::exchange(t.coroutine_, nullptr)) {}
		auto operator=(bg_task &&t) noexcept -> bg_task &
		{
			if (std::addressof(t) != this)
			{
				coroutine_ = std::exchange(t.coroutine_, nullptr);
			}
			return *this;
		}

        auto resume() -> void {
            if (coroutine_ != nullptr) {
                coroutine_.resume();
            }
        }

        operator std::coroutine_handle<>() const noexcept {
            return coroutine_;
        }

    private:
        coroutine_handle coroutine_{nullptr};
    };

    namespace detail
    {
        inline auto bg_promise::get_return_object() noexcept -> task_type {
            return task_type(coroutine_handle::from_promise(*this));
        }
    } // namespace detail
    

} // namespace coro