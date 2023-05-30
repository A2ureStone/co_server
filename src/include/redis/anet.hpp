#pragma once
#include "coro/net/io_scheduler.hpp"
#include "coro/net/socket.hpp"
#include "coro/task.hpp"

namespace redis
{
    using coro::task;
    using coro::net::io_scheduler;

    enum class net_status : int
    {
        ok = 0,
        // network error happen
        err = -1
    };

    /* return a tcpserver socket if success*/
    auto anet_tcpserver(io_scheduler &context, int port) -> coro::net::socket;

    /* read until count bytes or socket close or error. In former 2 cases, return number of read bytes. 
       In last case, we return the error code. */
   //  auto anet_read(io_scheduler &context, const coro::net::socket& sock, char *buf, int count) -> task<int>;

    /* write until count bytes or socket close or error. In former 2 cases, return number of write bytes. 
       In last case, we return the error code. */
   //  auto anet_write(io_scheduler &context, const coro::net::socket& sock, const char *buf, int count) -> task<int>;

    /* accept from the server socket, return client socket, fill the ip and port.
       if accept error, we return the error code. */
    auto anet_accept(io_scheduler &context, const coro::net::socket& serversock, std::string* ip, int *port) -> task<int>;

    auto anet_connect(io_scheduler &context, const std::string& ip, int port) -> task<coro::net::socket>;

} // namespace redis
