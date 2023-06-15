#pragma once
#include <list>
#include <chrono>
#include <coroutine>
#include <vector>

namespace coro
{
    namespace net
    {
        using time = std::chrono::system_clock::time_point;
        using duration = std::chrono::duration<int, std::milli>;

        struct timeout_item
        {
        public:
            time expire_time_;
            std::coroutine_handle<> coro_;
        };

        struct time_wheel
        {
        public:
            time_wheel();
            time_wheel(const time_wheel &) = delete;
            time_wheel(time_wheel &&) = delete;
            time_wheel &operator=(const time_wheel &) = delete;
            time_wheel &operator=(time_wheel &&) = delete;

            ~time_wheel() = default;

            auto add_timeout_item(time timeout_point, std::coroutine_handle<> coro) -> void;

            auto get_timeout_items() -> std::list<timeout_item>;

            static constexpr int64_t max_timeout_ = 40 * 1000;

        private:
            // max timeout 40s
            static constexpr int64_t max_size_ = 60 * 1000;

            time start_time_{};
            int64_t start_idx_{0};
            std::vector<std::list<timeout_item>> arr_{max_size_};
        };
    } // namespace net

} // namespace coro
