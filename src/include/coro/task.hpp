#pragma once
#include <coroutine>
#include <exception>
#include <utility>
#include <type_traits>
#include <iostream>

namespace coro
{
	// declare task for promise
	template <typename ReturnType = void>
	class task;

	namespace detail
	{
		struct promise_base
		{
			friend struct final_awaitable;
			struct final_awaitable
			{
				auto await_ready() const noexcept -> bool { return false; }

				// we need promise type info to construct promise
				template <class promise_type>
				auto await_suspend(std::coroutine_handle<promise_type> coro) const noexcept -> std::coroutine_handle<>
				{
					auto &promise = coro.promise();
					if (promise.continuation_ == nullptr)
					{
						// case for final task, use this trick to pop stack frame
						return std::noop_coroutine();
					}
					return promise.continuation_;
				}

				auto await_resume() const noexcept -> void {}
			};

			promise_base() noexcept = default;
			~promise_base() noexcept = default;

			// lazy execution
			auto initial_suspend() const noexcept {
				// std::cout << "start suspends\n";
				return std::suspend_always{};
			}

			auto final_suspend() const noexcept { return final_awaitable{}; }

			auto unhandled_exception() noexcept -> void { exception_ptr_ = std::current_exception(); }

			auto set_conotinuation(std::coroutine_handle<> continuation) noexcept -> void { continuation_ = continuation; }

		protected:
			std::coroutine_handle<> continuation_{nullptr};
			std::exception_ptr exception_ptr_{};
		};

		// only support value type
		template <class ReturnType>
		struct promise final : public promise_base
		{
			using coroutine_handle = std::coroutine_handle<promise>;
			using task_type = task<ReturnType>;

			promise() = default;
			~promise() = default;

			auto get_return_object() noexcept -> task_type;

			// first step of return value is copy, we must copy here, becauese the argument will destructor
			auto return_value(ReturnType res) -> void { res_ = std::move(res); }

			// wrapper of exception and result
			// we always move the res when fisrt of calling result
			// this method is for internal usage
			auto result() -> ReturnType &&
			{
				if (exception_ptr_ != nullptr)
				{
					std::rethrow_exception(exception_ptr_);
				}
				// cast to rvalue
				return std::move(res_);
			}

		private:
			ReturnType res_;
		};

		// specialize for no return value
		template <>
		struct promise<void> : public promise_base
		{
			using coroutine_handle = std::coroutine_handle<promise<void>>;
			using task_type = task<>;

			auto get_return_object() noexcept -> task_type;

			promise() noexcept = default;
			~promise() noexcept = default;

			auto return_void() noexcept -> void{};

			auto result() -> void 
			{
				if (exception_ptr_ != nullptr)
				{
					std::rethrow_exception(exception_ptr_);
				}
			}
		};
	} // namespace detail

	template <typename ReturnType>
	class [[nodiscard]] task
	{
	public:
		using promise_type = detail::promise<ReturnType>;
		using coroutine_handle = std::coroutine_handle<promise_type>;

		struct awaitable
		{
			awaitable(coroutine_handle coroutine) : coroutine_(coroutine) {}

			auto await_ready() const noexcept -> bool
			{
				return false;
			}

			auto await_suspend(std::coroutine_handle<> coro)
			{
				coroutine_.promise().set_conotinuation(coro);
				return coroutine_;
			};

			// ReturnType is not void, we return ReturnType&&
			auto await_resume() -> decltype(auto)
			{
				if constexpr (std::is_same_v<ReturnType, void>)
				{
					// propagate exception
					coroutine_.promise().result();
					return;
				} else {
					return coroutine_.promise().result();
				}
			}

		private:
			coroutine_handle coroutine_{nullptr};
		};

		task(coroutine_handle coroutine) noexcept : coroutine_(coroutine)
		{
		}

		task(task &&t) noexcept : coroutine_(std::exchange(t.coroutine_, nullptr)) {}

		auto operator=(task &&t) noexcept -> task &
		{
			if (std::addressof(t) != this)
			{
				if (coroutine_ != nullptr)
				{
					coroutine_.destroy();
				}
				coroutine_ = std::exchange(t.coroutine_, nullptr);
			}
			return *this;
		}

		~task()
		{
			if (coroutine_ != nullptr)
			{
				coroutine_.destroy();
			}
		}

		// delete copy operation
		task() = delete;
		task(const task &t) = delete;
		auto operator=(const task &t) -> task & = delete;

		auto is_ready() const noexcept -> bool
		{
			return coroutine_ == nullptr || coroutine_.done();
		}

		// fork primitive
		auto resume() -> bool
		{
			if (!is_ready())
			{
				coroutine_.resume();
			}
			return !coroutine_.done();
		}

		auto operator co_await() const noexcept
		{
			return awaitable{coroutine_};
		}

		// for debug case
		auto promise() -> promise_type& {
			return coroutine_.promise();
		}

	private:
		coroutine_handle coroutine_{nullptr};
	};

	namespace detail
	{
		template <class ReturnType>
		inline auto promise<ReturnType>::get_return_object() noexcept -> task_type
		{
			return task_type(coroutine_handle::from_promise(*this));
		}

		inline auto promise<void>::get_return_object() noexcept -> task_type
		{
			return task_type(coroutine_handle::from_promise(*this));
		}
	}
} // namespace coro