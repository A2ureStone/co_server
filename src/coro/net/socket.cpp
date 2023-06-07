#include "coro/net/socket.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <format>
#include <string.h>

namespace coro
{
    namespace net
    {
        auto socket::recv(void *buf, size_t nbytes) -> task<int>
        {
            co_return co_await context_ptr_->recv(fd_, buf, nbytes);
        }

        auto socket::send(const void *buf, size_t nbytes) -> task<int>
        {
            co_return co_await context_ptr_->send(fd_, buf, nbytes);
        }

        auto socket::read_until(char *buf, int count) -> task<int>
        {
            int nread, totlen = 0;
            while (totlen != count)
            {
                nread = co_await context_ptr_->recv(fd_, buf, count);
                totlen += nread;
                // socket close or error happens
                if (nread == 0)
                {
                    co_return totlen;
                }
                else if (nread < 0)
                {
                    co_return nread;
                }
            }
            co_return totlen;
        }

        auto socket::write_until(const char *buf, int count) -> task<int>
        {
            int nwrite, totlen = 0;
            while (totlen != count)
            {
                // this is io_uring, we will not get zero return value
                nwrite = co_await context_ptr_->send(native_handle(), buf, count);
                totlen += nwrite;
                // socket close or error happens
                if (nwrite == -1)
                {
                    co_return totlen;
                }
                else if (nwrite < -1)
                {
                    // handle eagain
                    if (nwrite == -EAGAIN)
                    {
                        std::cerr << "socket::write_until write return EAGAIN\n";
                        continue;
                    }
                    co_return nwrite;
                }
            }
            co_return totlen;
        }
    } // namespace net
} // namespace coro
