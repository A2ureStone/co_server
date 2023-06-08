#include "redis/redis_client.hpp"
#include "redis/utility.hpp"
#include <string.h>
#include <format>
#include <stdexcept>
#include <assert.h>
#include "redis/cmd.hpp"

namespace redis
{
    redis_client::redis_client(int fd, coro::net::io_scheduler *context_ptr, redis_server *svr_ptr) : cli_sock_(fd, context_ptr), context_ptr_(context_ptr), running_(true), svr_ptr_(svr_ptr)
    {
        // TODO a lots of member init
        select_db(0);
    }

    redis_client::~redis_client() {}

    // auto redis_client::read_coroutine(int fd, coro::net::io_scheduler *context_ptr, redis_server *svr_ptr) -> coro::bg_task<>
    auto redis_client::read_coroutine(std::shared_ptr<redis_client> cli) -> coro::bg_task<>
    {
        co_await cli->read_request_task();
        co_return;
    }

    auto redis_client::read_request_task() -> coro::task<void>
    {
        int nread, readlen;
        size_t qblen;
        while (running_)
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
            std::cout << std::format("read coro: recv bytes{}\n", nread);
            if (nread <= 0)
            {
                if (nread == 0)
                {
                    std::cerr << "Client closed connection\n";
                }
                else if (nread == -EAGAIN)
                {
                    // handle EAGAIN
                    // TODO current_client = NULL?
                    continue;
                }
                else
                {
                    std::cerr << std::format("Reading from client error, {}\n", strerror(-nread));
                }
                co_return;
            }

            // resize to correct len
            query_buf_.resize(qblen + nread);
            // TODO update heartbeat for client

            if (query_buf_.size() > svr_ptr_->client_max_querybuf_len_)
            {
                std::cerr << std::format("Closing client that reached max query buf len {}\n", svr_ptr_->client_max_querybuf_len_);
                co_return;
            }
            process_intput_buffer();
        }
        co_return;
    }

    auto redis_client::process_intput_buffer() -> void
    {
        // process while something in input buffer
        while (query_buf_.size())
        {
            // TODO client flags check
            // not in process request state
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
            // empty argv, reset
            if (argv_.empty())
            {
                reset_client();
            }
            else
            {
                // TODO process command
                if (process_cmd())
                {
                    reset_client();
                }
            }
        }
        return;
    }

    auto redis_client::process_oneline_buffer() -> op_status
    {
        size_t newline = 0;
        if (try_parse_oneline(newline) != op_status::redis_ok)
        {
            return op_status::redis_err;
        }

        auto querylen = newline;
        // this is not inplace split
        argv_ = redis::utility::str_split(query_buf_.substr(0, querylen));
        // remove the first line from the buffer
        query_buf_.erase(0, querylen + 2);
        return op_status::redis_ok;
    }

    auto redis_client::process_multibulk_buffer() -> op_status
    {
        // we don't read the bulk len
        if (multi_bulklen_ == 0)
        {
            // assert client is in reset state
            assert(argv_.empty());

            // read multibulk len
            size_t newline = 0;
            if (try_parse_oneline(newline) != op_status::redis_ok)
            {
                return op_status::redis_err;
            }

            // we are in bulk read, first must be '*'
            assert(query_buf_[0] == '*');
            bool parse_success = false;
            int multi_bulklen = -1;
            try
            {
                // this is not convert inplace
                multi_bulklen = std::stoi(query_buf_.substr(1, newline));
                if (multi_bulklen <= 1024 * 1024)
                {
                    parse_success = true;
                }
            }
            catch (const std::invalid_argument &ex)
            {
                add_reply("Protocol error: parse bulk length fail\n");
            }
            catch (const std::out_of_range &ex)
            {
                add_reply("Protocol error: invalid bulk length\n");
            }
            if (!parse_success)
            {
                set_protocol_error();
                return op_status::redis_err;
            }
            // consume the buffer
            query_buf_.erase(0, newline + 2);
            // if <= 0, we success parse it
            if (multi_bulklen <= 0)
            {
                return op_status::redis_ok;
            }
            multi_bulklen_ = multi_bulklen;
            // is this neccessary?
            // if (!argv_.empty())
            // {
            //     argv_.clear();
            // }

            // we need this to bypass reset state
            argv_.resize(multi_bulklen_);
        }
        assert(multi_bulklen_ > 0);
        // enter read bulk state
        // std::cout << std::format("multi_bulk: {}\n", multi_bulklen_);
        while (multi_bulklen_ > 0)
        {
            // std::cout << std::format("bulk: {}\n", bulklen_);
            if (bulklen_ == -1)
            {
                // try to read bulklen
                size_t newline = 0;
                if (try_parse_oneline(newline) != op_status::redis_ok)
                {
                    return op_status::redis_err;
                }

                if (query_buf_[0] != '$')
                {
                    // TODO error reply
                    add_reply("Protocol error: too big bulk count string\n");
                    set_protocol_error();
                    return op_status::redis_err;
                }

                bool parse_success = false;
                try
                {
                    int bulklen = std::stoi(query_buf_.substr(1, newline));
                    if (bulklen >= 0 && bulklen <= 512 * 1024 * 1024)
                    {
                        parse_success = true;
                    }
                    query_buf_.erase(0, newline + 2);
                    // TODO large bulk object optimization
                    bulklen_ = bulklen;
                }
                catch (const std::invalid_argument &ex)
                {
                    add_reply("Protocol error: parse bulk length fail\n");
                }
                catch (const std::out_of_range &ex)
                {
                    add_reply("Protocol error: invalid bulk length\n");
                }
                if (!parse_success)
                {
                    // TODO add reply
                    set_protocol_error();
                    return op_status::redis_err;
                }
            }
            // peek bulklen_ bytes
            if ((query_buf_.size()) < static_cast<size_t>(bulklen_ + 2))
            {
                // not enough bytes
                break;
            }
            else
            {
                // reset to read bulk state
                // TODO optimization
                argv_[argv_.size() - multi_bulklen_] = query_buf_.substr(0, bulklen_);
                query_buf_.erase(0, bulklen_ + 2);
                multi_bulklen_--;
                bulklen_ = -1;
            }
        }
        // read all bulk
        if (multi_bulklen_ == 0)
        {
            return op_status::redis_ok;
        }
        return op_status::redis_err;
    }

    auto redis_client::try_parse_oneline(size_t &newline) -> op_status
    {
        newline = query_buf_.find("\r\n");
        if (newline == std::string::npos)
        {
            if (query_buf_.size() > REDIS_INLINE_MAX_SIZE)
            {
                // TODO add reply to client
                add_reply("Protocol error: too big inline request\n");
                set_protocol_error();
            }
            return op_status::redis_err;
        }
        return op_status::redis_ok;
    }

    auto redis_client::write_coroutine(std::shared_ptr<redis_client> cli) -> coro::bg_task<>
    {
        co_await cli->send_reply_task();
    }

    /* coroutine to send reply to client */
    auto redis_client::send_reply_task() -> coro::task<>
    {
        // int nwritten = 0, totwritten = 0;
        int nwritten = 0;
        while (!reply_.empty())
        {
            std::string &reply_str = reply_.front();
            int reply_len = reply_str.size();

            nwritten = co_await cli_sock_.write_until(reply_str.data(), reply_len);
            // error happen or socket close(EAGAIN is already handled in write_until)
            if (nwritten != reply_len)
            {
                std::cerr << std::format("Error writing to client ", strerror(-nwritten));
                running_ = false;
                // this will wake up read coroutine and free the client
                co_await context_ptr_->notify_on_fd(cli_sock_.native_handle());
                co_return;
            }

            totwritten_ += nwritten;
            reply_.pop_front();
        }
        if (nwritten > 0)
        {
            lastinteraction_ = std::chrono::system_clock::now();
        }
        // all error capture in the while loop
        assert(reply_.empty());
        /* Close connection after entire reply has been sent. */
        /* we use shared_ptr to free redis_client, notify for the case read coroutine is sleeping */
        if (flags_ & REDIS_CLOSE_AFTER_REPLY)
        {
            running_ = false;
            // this will wake up read coroutine and free the client
            co_await context_ptr_->notify_on_fd(cli_sock_.native_handle());
        }
        co_return;
    }

    /* add message to send buffer */
    auto redis_client::add_reply(std::string message) -> void
    {
        // TODO replstate
        if (reply_.empty())
        {
            try
            {
                context_ptr_->spawn(write_coroutine(shared_from_this()));
            }
            catch (const std::exception &e)
            {
                std::cout << e.what() << std::endl;
                throw;
            }
        }
        reply_.emplace_back(std::move(message));
        // TODO refcount ???
    }

    auto redis_client::reset_client() -> void
    {
        argv_.clear();
        cmd_ = nullptr;
        reqtype_ = req_type::empty;
        multi_bulklen_ = 0;
        bulklen_ = -1;
        // TODO flags clear
    }

    auto redis_client::set_protocol_error() -> void
    {
        // TODO log support
        flags_ |= REDIS_CLOSE_AFTER_REPLY;
    }

    auto redis_client::process_cmd() -> bool
    {
        auto cmd = cmd_table.lookup_cmd(argv_[0]);
        if (!cmd)
        {
            add_reply("-ERR unknown command\r\n");
            return true;
        }
        else if (cmd->arity_ > 0 && cmd->arity_ != static_cast<int>(argv_.size()))
        {
            add_reply("-ERR wrong number of arguments\r\n");
            return true;
        }
        cmd->execute(*this);
        return true;
    }

    auto redis_client::select_db(int id) -> bool
    {
        if (id < 0 || id > svr_ptr_->db_num_)
        {
            return false;
        }
        db_ = &svr_ptr_->dbs_[id];
        return true;
    }

} // namespace redis
