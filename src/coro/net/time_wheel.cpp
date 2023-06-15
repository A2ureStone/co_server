#include "coro/net/time_wheel.hpp"
#include <utility>
#include <iostream>

namespace coro
{
    namespace net
    {
        time_wheel::time_wheel() : start_time_(std::chrono::system_clock::now())
        {
        }

        auto time_wheel::add_timeout_item(time timeout_point, std::coroutine_handle<> coro) -> void
        {
            auto index = std::chrono::duration_cast<std::chrono::milliseconds>(timeout_point - start_time_).count();
            if (index > static_cast<int64_t>(arr_.size()))
            {
                std::cerr << "add_timeout_item: low accuracy warning\n";
                index = arr_.size() - 1;
            }
            arr_[(start_idx_ + index) % arr_.size()].emplace_back(timeout_point, coro);
        }

        auto time_wheel::get_timeout_items() -> std::list<timeout_item>
        {
            auto cur_time = std::chrono::system_clock::now();
            auto timeout_nums = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - start_time_).count() + 1;
            timeout_nums = std::min(timeout_nums, static_cast<long>(arr_.size()));
            std::list<timeout_item> timeout_items;
            for (int64_t i = 0; i < timeout_nums; ++i)
            {
                timeout_items.splice(timeout_items.end(), std::exchange(arr_[(start_idx_ + i) % arr_.size()], std::list<timeout_item>{}));
            }
            start_idx_ = (start_idx_ + timeout_nums - 1) % arr_.size();
            start_time_ = cur_time;

            return timeout_items;
        }
    } // namespace net

} // namespace coro
