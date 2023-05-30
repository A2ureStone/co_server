#pragma once
#include <arpa/inet.h>
#include <array>
#include <string>
#include <stdexcept>
#include <cstring>

namespace coro::net {
    enum class domain_t : int
    {
        ipv4 = AF_INET
    };

    class ip_address {
    public:
        static constexpr size_t ipv4_len{4};

        ip_address() = default;

        ip_address(const ip_address&) = default;
        ip_address(ip_address&&) = default;
        ip_address &operator=(const ip_address&) = default;
        ip_address &operator=(ip_address&&) = default;

        auto domain() const noexcept -> domain_t { return domain_; }

        static auto from_string(const std::string &address, domain_t domain = domain_t::ipv4) -> ip_address
        {
            ip_address addr{};
            addr.domain_ = domain;
            int success = inet_pton(static_cast<int>(domain), address.data(), addr.ip_.data());
            if (success != 1)
            {
                throw std::runtime_error{"coro::net::ip_address failed to convert address string"};
            }
            return addr;
    }

    auto to_string() const -> std::string {
        std::string res;
        if (domain_ == domain_t::ipv4) {
            res.resize(INET_ADDRSTRLEN, '\0');
        }
        const char *success = inet_ntop(static_cast<int>(domain_), ip_.data(), res.data(), res.size());
        if (success == nullptr) {
            throw std::runtime_error{"coro::net::ipaddress conver to address string failed"};
        }
        return res;
    }

    private: 
        domain_t domain_{domain_t::ipv4};
        std::array<uint8_t, ipv4_len> ip_;
    };
}