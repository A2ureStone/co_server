#include "redis/redis_server.hpp"
#include <exception>
#include <format>

int main(int argc, char **argv)
{
    try
    {
        // TODO program param parse
        redis::redis_server svr;
        svr.run();
        // TODO load rdb
    }
    catch (const std::bad_alloc &e)
    {
        std::cerr << std::format("exception: {}\n", e.what());
    }
    return 0;
}