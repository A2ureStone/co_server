#include "redis/anet.hpp"
#include "coro/net/socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <format>
#include <string.h>

namespace redis
{
    auto anet_tcpserver(io_scheduler& context, int port) -> coro::net::socket
    {
        int s, on = 1;
        sockaddr_in sa;
        if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            std::cerr << std::format("anet_tcpserver: make socket error, {}", strerror(errno));
            return coro::net::socket();
        }
        // raii close
        coro::net::socket sock(s, &context);
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        {
            std::cerr << std::format("anet_tcpserver: set socket error, {}", strerror(errno));
            return coro::net::socket();
        }
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = ntohs(port);
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) == -1) {
            std::cerr << std::format("anet_tcpserver: bind socket error, {}", strerror(errno));
            return coro::net::socket();
        } 
        if (listen(s, 64) == -1) {
            std::cerr << std::format("anet_tcpserver: listen socket error, {}", strerror(errno));
            return coro::net::socket();
        }
        // copy elison
        return sock;
    }

    auto anet_connect(io_scheduler &context, const std::string& ip, int port) -> task<coro::net::socket> {
        int s, on = 1;
        sockaddr_in sa;
        if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            std::cerr << std::format("anet_connect: make socket error, {}", strerror(errno));
            co_return coro::net::socket();
        }
        // raii close
        coro::net::socket sock(s, &context);
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
        {
            std::cerr << std::format("anet_connect: set socket error, {}", strerror(errno));
            co_return coro::net::socket();
        }
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = ntohs(port);
        if (inet_aton(ip.c_str(), &sa.sin_addr) == 0) {
            std::cerr << std::format("anet_connect: ip convert error, {}", strerror(errno));
            co_return coro::net::socket();
        }
        int res;
        if ((res = co_await context.connect(s, reinterpret_cast<sockaddr *>(&sa), sizeof(sa))) == -1)
        {
            std::cerr << std::format("anet_connect: connect error, {}", strerror(-res));
            co_return coro::net::socket();
        }
        co_return std::move(sock);
    }

    auto anet_accept(io_scheduler &context, const coro::net::socket& serversock, std::string* ip, int *port) -> task<int> {
        int fd;
        struct sockaddr_in sa;
        socklen_t sa_len = sizeof(sa);

        while (1) {
            fd = co_await context.accept(serversock.native_handle(), reinterpret_cast<sockaddr *>(&sa), &sa_len);
            if (fd == -1) {
                if (fd == -EINTR) {
                    continue;
                } else {
                    std::cerr << std::format("anet_accept: get error, {}\n", strerror(-fd));
                    co_return fd;
                }
            }
            break;
        }
        if (ip)
            *ip = std::string(inet_ntoa(sa.sin_addr));
        if (port)
            *port = ntohs(sa.sin_port);
        co_return fd;
    }

} // namespace redis