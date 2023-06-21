#pragma once
#ifndef UTIL_IP_H
#define UTIL_IP_H

#include <cstdint>

#include <array>

#include <unistd.h>     // close(int)

#include <netinet/in.h> // IPPROTO_TCP IPPROTO_UDP AF_INET AF_INET6 SOCK_STREAM SOCK_DGRAM

#include "Channel.h"
#include "Socket.h"
#include "Tools.h"
#include "base/Acceptor.h"
#include "base/EndPoint.h"

namespace wutils::network::ip {

class V4 {
public:
    static int Family() { return AF_INET; }
    using native_data_type            = ::in_addr;
    using native_sockaddr_type        = sockaddr_in;
    static constexpr int sockaddr_len = sizeof(native_sockaddr_type);
    using SockAddr                    = std::array<uint8_t, sockaddr_len>;

    /**
     * @example
     * V4::Address e1();
     * e1.Assign("127.0.0.1"); // true
     * e1.Assign("::1"); //false
     */
    class Address {
    public:
        using Family = V4;

        Address() = default;
        explicit Address(const std::string &ip) { is_available_ = IpStrToAddr(ip, &this->data_); }
        virtual ~Address() = default;

        bool Assign(const std::string &ip) {
            is_available_ = IpStrToAddr(ip, &this->data_);
            return is_available_;
        }
        bool Assign(const native_data_type &in) {
            this->data_   = in;
            is_available_ = true;
            return is_available_;
        }

        operator bool() const { return this->is_available_; }
        const native_data_type *AsInAddr() const { return &this->data_; };
        std::string             AsString() const {
            std::string ip;
            if(is_available_) {
                IpAddrToStr(this->data_, ip);
            }
            return ip;
        }

    private:
        native_data_type data_{0};
        bool             is_available_{false};
    };

    class EndPointInfo {};
};


class V6 {
public:
    static int Family() { return AF_INET6; }
    using in_addr                 = in6_addr;
    using addr                    = sockaddr_in6;
    static constexpr int addr_len = sizeof(addr);
    using Addr                    = std::array<uint8_t, addr_len>;
};

} // namespace wutils::network::ip


#endif // UTIL_IP_H
