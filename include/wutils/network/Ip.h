#pragma once
#ifndef UTIL_IP_H
#define UTIL_IP_H

#include <cstdint>

#include <array>

#include <unistd.h>     // close(int)

#include <netinet/in.h> // IPPROTO_TCP IPPROTO_UDP AF_INET AF_INET6 SOCK_STREAM SOCK_DGRAM

// #include "Channel.h"
#include "Socket.h"
#include "Tools.h"
#include "base/Acceptor.h"
#include "base/EndPoint.h"

namespace wutils::network::ip {

template <class FAMILY>
class IPEndPointInfo : public IEndPointInfo<FAMILY> {
public:
    IPEndPointInfo() : IEndPointInfo<FAMILY>() {}
    IPEndPointInfo(const typename FAMILY::Address &addr, uint16_t port) { this->Assign(addr, port); }

    bool Assign(const typename FAMILY::Address &addr, uint16_t port) {
        assert(addr);
        this->is_valid_ = GetSockAddr_in(addr.AsInAddr(), port, this->AsSockAddr());
        return this->is_valid_;
    };
};

class v4 {
public:
    static int Family() { return AF_INET; }
    using native_data_type            = ::in_addr;
    using native_sockaddr_type        = sockaddr_in;
    static constexpr int sockaddr_len = sizeof(native_sockaddr_type);
    using SockAddr                    = std::array<uint8_t, sockaddr_len>;

    /**
     * @example
     * v4::Address e1(); \n
     * e1.Assign("127.0.0.1"); // true \n
     * e1.Assign("::1"); //false
     */
    using Address = IAddress<v4>;
    /*class Address {
    public:
        using Family = v4;

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
    };*/

    class EndPointInfo : public IPEndPointInfo<v4> {
    public:
        EndPointInfo() : IPEndPointInfo<v4>() {}
        EndPointInfo(const Address &addr, uint16_t port) : IPEndPointInfo<v4>(addr, port) {}

        using IEndPointInfo::Assign;
        using IPEndPointInfo::Assign;

        std::tuple<std::string, uint16_t> Dump() {
            auto        native_struct = (native_sockaddr_type *)this->sockaddr_.data();
            std::string ip;
            uint16_t    port;

            IpAddrToStr(native_struct->sin_addr, ip);
            NtoHS(native_struct->sin_port, &port);

            return {ip, port};
        }
    };
};


class v6 {
public:
    static int Family() { return AF_INET6; }
    using native_data_type            = in6_addr;
    using native_sockaddr_type        = sockaddr_in6;
    static constexpr int sockaddr_len = sizeof(native_sockaddr_type);
    using SockAddr                    = std::array<uint8_t, sockaddr_len>;

    /**
     * @example
     * v6::Address e1(); \n
     * e1.Assign("127.0.0.1"); // false \n
     * e1.Assign("::1"); //true
     */
    using Address = IAddress<v6>;

    class EndPointInfo : public IPEndPointInfo<v6> {
    public:
        EndPointInfo() : IPEndPointInfo<v6>() {}
        EndPointInfo(const Address &addr, uint16_t port) : IPEndPointInfo<v6>(addr, port) {}

        using IEndPointInfo::Assign;
        using IPEndPointInfo::Assign;
        std::tuple<std::string, uint16_t> Dump() {
            auto        native_struct = (native_sockaddr_type *)this->sockaddr_.data();
            std::string ip;
            uint16_t    port;

            IpAddrToStr(native_struct->sin6_addr, ip);
            NtoHS(native_struct->sin6_port, &port);

            return {ip, port};
        }
    };
};

} // namespace wutils::network::ip


#endif // UTIL_IP_H
