#pragma once
#ifndef UTIL_IP_H
#define UTIL_IP_H

#include <cstdint>

#include <array>

#include <unistd.h> // close(int)

#include <cassert>
#include <netinet/in.h> // IPPROTO_TCP IPPROTO_UDP AF_INET AF_INET6 SOCK_STREAM SOCK_DGRAM

// #include "Channel.h"
#include "Tools.h"
#include "base/EndPoint.h"
#include "base/IAcceptor.h"
#include "wutils/network/base/ISocket.h"

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
    static int               Family() { return AF_INET; }
    static const std::string LOOK_BACK;

    using sockaddr_t                  = sockaddr_in;
    using address_t                   = typeof(sockaddr_t::sin_addr);
    static constexpr int sockaddr_len = sizeof(sockaddr_t);

    /**
     * @example
     * v4::Address e1(); \n
     * e1.Assign("127.0.0.1"); // true \n
     * e1.Assign("::1"); //false
     */
    using Address = IAddress<v4>;

    class EndPointInfo : public IPEndPointInfo<v4> {
    public:
        EndPointInfo() : IPEndPointInfo<v4>() {}
        EndPointInfo(const Address &addr, uint16_t port) : IPEndPointInfo<v4>(addr, port) {}

        using IEndPointInfo::Assign;
        using IPEndPointInfo::Assign;

        std::tuple<std::string, uint16_t> Dump() {
            auto        native_struct = (sockaddr_t *)this->sockaddr_.data();
            std::string ip;
            uint16_t    port;

            IpAddrToStr(native_struct->sin_addr, ip);
            NtoHS(native_struct->sin_port, &port);

            return {ip, port};
        }
    };
};
const std::string v4::LOOK_BACK = "127.0.0.1";

class v6 {
public:
    static int         Family() { return AF_INET6; }
    static std::string LOOK_BACK() { return "::1"; }

    using sockaddr_t                  = sockaddr_in6;
    using address_t                   = typeof(sockaddr_t::sin6_addr);
    static constexpr int sockaddr_len = sizeof(sockaddr_t);

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
            auto        native_struct = (sockaddr_t *)this->sockaddr_.data();
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
