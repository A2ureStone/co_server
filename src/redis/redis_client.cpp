#include "redis/redis_client.hpp"
#include "redis/utility.hpp"
#include <string.h>
#include <format>

namespace redis
{
    redis_client::redis_client(int fd, coro::net::io_scheduler *context_ptr) : cli_sock_(fd, context_ptr)
    {
        // TODO a lots of member init
    }

    auto redis_client::client_task() -> coro::bg_task<>
    {
        int nread, readlen;
        size_t qblen;
        while (true)
        {
            readlen = REDIS_IOBUF_LEN;
            // TODO Check for multi bulk request
            qblen = query_buf_.size();
            // seems no need
            if (query_buf_peek_ < qblen)
            {
                query_buf_peek_ = qblen;
            }
            query_buf_.resize(qblen + readlen);
            nread = co_await cli_sock_.recv(&query_buf_.front() + qblen, readlen);
            if (nread > 0)
            {
                query_buf_.resize(qblen + nread);
                // TODO update heartbeat for client
                if (query_buf_.size() > svr_ptr_->client_max_querybuf_len_)
                {
                    std::cerr << std::format("Closing client that reached max query buf len {}\n", svr_ptr_->client_max_querybuf_len_);
                    co_return;
                }
            }
            else if (nread == 0)
            {
                co_return;
            }
            else if (nread != -EAGAIN)
            {
                continue;
            }
            else
            {
                std::cerr << std::format("Reading from client error, {}", strerror(-nread));
                co_return;
            }
        }
        co_return;
    }

    auto redis_client::process_intput_buffer() -> coro::task<>
    {
        // process while something in input buf
        while (query_buf_.size())
        {
            // TODO client flags check
            if (reqtype_ == req_type::empty)
            {
                if (query_buf_[0] == '*')
                {
                    reqtype_ = req_type::multibulk;
                }
                else
                {
                    reqtype_ = req_type::oneline;
                }
            }
            if (reqtype_ == req_type::oneline)
            {
                if (process_oneline_buffer() != op_status::redis_ok)
                    break;
            }
            else
            {
                if (process_multibulk_buffer() != op_status::redis_ok)
                    break;
            }
            if (argc_ == 0) {
                // TODO reset client state
            } else {
                // TODO process command
            }
        }
        co_return;
    }

    auto redis_client::process_oneline_buffer() -> op_status
    {
        auto newline = query_buf_.find("\r\n");
        if (newline == std::string::npos) {
            if (query_buf_.size() > REDIS_INLINE_MAX_SIZE) {
            // TODO add reply to client
            }
            return op_status::redis_err;
        }

        auto querylen = newline;
        // this is not inplace split
        argv_ = redis::utility::str_split(query_buf_.substr(0, query_len));
        query_buf_.erase(0, querylen+2);
        return op_status::redis_ok;
    }

    auto redis_client::process_multibulk_buffer() -> op_status {
        return op_status::redis_ok;
    }

} // namespace redis
