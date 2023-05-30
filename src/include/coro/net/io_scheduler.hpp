#pragma once
#include <thread>
#include "coro/task.hpp"
#include "coro/net/poll_info.hpp"
#include <liburing.h>
#include <vector>
#include <sys/socket.h>

namespace coro
{
    namespace net
    {
        class io_scheduler
        {
        public:
            io_scheduler();
            ~io_scheduler();


            // sync_wait bg_coroutine to do network
            auto run() -> void;

            [[nodiscard]] auto recv(int fd, void *buf, size_t nbytes) -> coro::task<int>;

            [[nodiscard]] auto accept(int fd, sockaddr* addr, socklen_t* addr_len) -> coro::task<int>;

            [[nodiscard]] auto connect(int fd, const sockaddr* addr, socklen_t addr_len) -> coro::task<int>;

            [[nodiscard]] auto send(int fd, const void *buf, size_t nbytes) -> coro::task<int>;

            // start a coroutine in backgroud, choose lazy run or eager run
            auto spawn(std::coroutine_handle<> coro) -> void {
                // if (!lazy) {
                //     coro.resume();
                // }
                handles_to_resume_.emplace_back(coro);
            }

            // a thread singleton to get io_scheduler to yield
        private:
            // bool looping_{false};
            // const pid_t threadId_{0};
            static constexpr int queue_depth_ = 4112;
            struct io_uring ring_;
            std::vector<std::coroutine_handle<>> handles_to_resume_{};
            size_t waiting_ev_{0};
            static constexpr int ring_cqe_num_ = 128;
        };
    }
}