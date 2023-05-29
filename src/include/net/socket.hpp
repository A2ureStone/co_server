#pragma once

namespace coro::net {
    class socket {
    public:
        enum class type_t
        {
            tcp
        };

        enum class blocking_t
        {
            yes,
            no
        };

        struct options {
        };

    private:
        int fd_{-1};
    };
}