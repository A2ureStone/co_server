#include "coro/sync_wait.hpp"

namespace coro {
    namespace details {
        auto sync_wait_event::set() noexcept -> void {
            {
                std::lock_guard<std::mutex> lk{mutex_};
                set_ = true;
            }
            cv_.notify_all();
        }
        auto sync_wait_event::reset() noexcept -> void {
            std::lock_guard<std::mutex> lk{mutex_};
            set_ = false;
        }
        auto sync_wait_event::wait() noexcept -> void {
            std::unique_lock<std::mutex> lk{mutex_};
            cv_.wait(lk, [this]
                     { return set_ == true; });
            // after wakeup, we own the lock
        }
    }
}