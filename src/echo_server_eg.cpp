#include "net/io_scheduler.hpp"
#include "coro/bg_task.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <atomic>

using coro::bg_task;
using coro::net::io_scheduler;

auto client = [](io_scheduler &context, int client_fd) -> bg_task<>
{
    char buf[4096];
    int read_num = 0;
    while ((read_num = co_await context.recv(client_fd, buf, 4096)) > 0)
    {
        if (co_await context.send(client_fd, buf, read_num) <= 0)
        {
            std::cerr << "send error\n";
            break;
        }
    }
    close(client_fd);
    co_return;
};

auto server = [](io_scheduler &context) -> bg_task<>
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    int port = 8086;
    if (sock == -1)
    {
        std::cerr << "sock init error\n";
        co_return;
    }
    struct sockaddr_in svr_addr;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(port);
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, reinterpret_cast<const sockaddr *>(&svr_addr), sizeof(svr_addr)) < 0)
    {
        std::cerr << "bind error\n";
        co_return;
    }
    if (listen(sock, 128) < 0)
    {
        std::cerr << "listen error\n";
        co_return;
    }
    std::cout << "server start at port " << port << std::endl;
    int conn = 0;
    int all = 10000;
    while (conn < all)
    {
        int client_fd = co_await context.accept(sock, nullptr, nullptr);
        // std::cout << "server accept " << client_fd << std::endl;
        conn++;
        if (client_fd < 0)
        {
            std::cerr << "accept fail\n";
            co_return;
        }
        // conn++;
        context.spawn(client(context, client_fd));
        // std::cout << "conn " << conn << std::endl;
        // std::cout << "total " << total << std::endl;
    }
    close(sock);
    co_return;
};

int main()
{
    io_scheduler context;
    context.spawn(server(context));
    context.run();
    return 0;
}