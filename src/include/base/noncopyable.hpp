#pragma once

namespace coro
{
    class noncopyable
    {
    protected:
        noncopyable() = default;
        ~noncopyable() = default;

    public:
        noncopyable(const noncopyable &) = delete;
        noncopyable& operator=(const noncopyable &) = delete;
    };
} // namespace coro
