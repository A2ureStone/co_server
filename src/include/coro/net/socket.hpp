#pragma once
#include <sys/socket.h>
#include <unistd.h>
#include "coro/net/io_scheduler.hpp"

namespace coro
{
    namespace net
    {
        class socket
        {
        public:
            enum class type_t
            {
                noop,
                tcp_server,
                tcp_client
            };

            enum class blocking_t
            {
                yes,
                no
            };

            socket() = default;
            explicit socket(int fd, io_scheduler *context_ptr) : fd_(fd), context_ptr_(context_ptr){};
            socket(const socket &) = delete;
            socket(socket &&other) : fd_(std::exchange(other.fd_, -1)), context_ptr_(other.context_ptr_){}
            auto operator=(const socket &) -> socket & = delete;
            auto operator=(socket &&other) -> socket &
            {
                if (std::addressof(other) != this)
                {
                    if (fd_ != -1)
                    {
                        close(fd_);
                    }
                    fd_ = std::exchange(other.fd_, -1);
                    context_ptr_ = other.context_ptr_;
                    // type_ = other.type_;
                }
                return *this;
            }
            ~socket()
            {
                if (fd_ != -1)
                {
                    close(fd_);
                }
            }

            /* is the socket has correct fd */
            auto is_init() const noexcept -> bool { return fd_ != -1; }

            /* return fd */
            auto native_handle() const noexcept -> int { return fd_; }

            /* set type for the socket, client or server */
            // auto set_type(type_t type) -> void { type_ = type; }

            /* same as system call, error code is -(return_val) if happens */
            auto recv(void *buf, size_t nbytes) -> task<int>;

            /* same as system call, error code is -(return_val) if happens */
            auto send(const void *buf, size_t nbytes) -> task<int>;

            /* read until count bytes or socket close or error. In former 2 cases, return number of read bytes.
               In last case, we return the error code. */
            auto read_until(char *buf, int count) -> coro::task<int>;

            /* write until count bytes or socket close or error. In former 2 cases, return number of write bytes.
               In last case, we return the error code. */
            auto write_until(const char *buf, int count) -> coro::task<int>;

            // static auto make_socket() -> socket;


        private:
            int fd_{-1};
            io_scheduler *context_ptr_{nullptr};
            // type_t type_{type_t::noop};
        };
    }
}
