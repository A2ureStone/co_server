#include "net/io_scheduler.hpp"
#include <string.h>

namespace coro
{
    namespace net
    {
        io_scheduler::io_scheduler()
        {
            io_uring_queue_init(queue_depth_, &ring_, 0);
        }

        io_scheduler::~io_scheduler()
        {
            io_uring_queue_exit(&ring_);
        }

        auto io_scheduler::run() -> void
        {
            struct io_uring_cqe *cqe;
            // 1ms for timeout
            __kernel_timespec timeout{.tv_sec = 0, .tv_nsec = 1000000};
            // __kernel_timespec timeout{.tv_sec = 1, .tv_nsec = 0};
            struct io_uring_cqe *cqes[ring_cqe_num_];
            std::vector<std::coroutine_handle<>> tasks;
            while (waiting_ev_ != 0 || !handles_to_resume_.empty())
            {
                if (!handles_to_resume_.empty())
                {
                    // swap in case of adding while loop
                    tasks.swap(handles_to_resume_);
                    for (auto handle : tasks)
                    {
                        handle.resume();
                    }
                    tasks.clear();
                }

                int res = io_uring_wait_cqe_timeout(&ring_, &cqe, &timeout);
                if (res == 0)
                {
                    int cqe_count = io_uring_peek_batch_cqe(&ring_, cqes, ring_cqe_num_);
                    for (int i = 0; i < cqe_count; ++i)
                    {
                        cqe = cqes[i];
                        poll_info *pi = reinterpret_cast<poll_info *>(cqe->user_data);
                        pi->res_ = cqe->res;
                        handles_to_resume_.emplace_back(pi->awaiting_coro_);
                    }
                    waiting_ev_ -= cqe_count;
                    io_uring_cq_advance(&ring_, static_cast<unsigned int>(cqe_count));
                }
                else if (-res != ETIME)
                {
                    std::cerr << "io_scheduler::run() io_uring_wait_cqe error " << res << std::endl;
                    std::cerr << strerror(-res) << std::endl;
                    // std::cerr << "waiting " << waiting_ev_ << std::endl;
                    exit(1);
                }
            }
        }

        [[nodiscard]] auto io_scheduler::recv(int fd, void *buf, size_t nbytes) -> coro::task<int>
        {
            struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
            if (sqe == nullptr) {
                std::cerr << "io_uring have no sqe\n";
                co_return -1;
            }
            auto pi = poll_info{};
            pi.iov_.iov_base = buf;
            pi.iov_.iov_len = nbytes;
            io_uring_prep_readv(sqe, fd, &pi.iov_, 1, 0);
            io_uring_sqe_set_data(sqe, &pi);
            io_uring_submit(&ring_);
            waiting_ev_++;
            co_await pi;
            // read complete
            co_return pi.res_;
        }

        [[nodiscard]] auto io_scheduler::accept(int fd, sockaddr* addr, socklen_t* addr_len) -> coro::task<int>
        {
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
            if (sqe == nullptr) {
                std::cerr << "io_uring have no sqe\n";
                co_return -1;
            }
            auto pi = poll_info{};
            io_uring_prep_accept(sqe, fd, addr, addr_len, 0);
            io_uring_sqe_set_data(sqe, &pi);
            io_uring_submit(&ring_);
            waiting_ev_++;
            co_await pi;
            co_return pi.res_;
        }

        [[nodiscard]] auto io_scheduler::connect(int fd, const sockaddr* addr, socklen_t addr_len) -> coro::task<int> {
            struct io_uring_sqe *sqe = io_uring_get_sqe(&ring_);
            if (sqe == nullptr) {
                std::cerr << "io_uring have no sqe\n";
                co_return -1;
            }
            auto pi = poll_info{};
            io_uring_prep_connect(sqe, fd, addr, addr_len);
            io_uring_sqe_set_data(sqe, &pi);
            io_uring_submit(&ring_);
            waiting_ev_++;
            co_await pi;
            co_return pi.res_;
        }

        [[nodiscard]] auto io_scheduler::send(int fd, const void *buf, size_t nbytes) -> coro::task<int> {
            struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
            if (sqe == nullptr) {
                std::cerr << "io_uring have no sqe\n";
                co_return -1;
            }
            auto pi = poll_info{};
            pi.iov_.iov_base = const_cast<void*>(buf);
            pi.iov_.iov_len = nbytes;
            io_uring_prep_writev(sqe, fd, &pi.iov_, 1, 0);
            io_uring_sqe_set_data(sqe, &pi);
            io_uring_submit(&ring_);
            waiting_ev_++;
            co_await pi;
            // read complete
            co_return pi.res_;
        }

    } // namespace net

} // namespace coro