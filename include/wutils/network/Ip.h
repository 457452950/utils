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
    using in_addr                 = ::in_addr;
    using addr                    = sockaddr_in;
    static constexpr int addr_len = sizeof(addr);
    using Addr                    = std::array<uint8_t, addr_len>;

    using Address = ADDRESS<V4>;

    class EndPointInfo : public ENDPOINTINFO<V4> {
    public:
        EndPointInfo() : ENDPOINTINFO<V4>() {}
        EndPointInfo(const Address &address, uint16_t port) : ENDPOINTINFO<V4>() {
            GetSockAddr_in(address.AsInAddr(), port, (addr *)this->addr_.data());
        }
        bool Assign(const Address &address, uint16_t port) {
            return GetSockAddr_in(address.AsInAddr(), port, (addr *)this->addr_.data());
        }
    };
};

class V6 {
public:
    static int Family() { return AF_INET6; }
    using in_addr                 = in6_addr;
    using addr                    = sockaddr_in6;
    static constexpr int addr_len = sizeof(addr);
    using Addr                    = std::array<uint8_t, addr_len>;

    using Address = ADDRESS<V6>;
    class EndPointInfo : ENDPOINTINFO<V6> {
    public:
        EndPointInfo(const Address &address, uint16_t port) : ENDPOINTINFO<V6>() {
            GetSockAddr_in(address.AsInAddr(), port, (addr *)this->addr_.data());
        }
        bool Assign(const Address &address, uint16_t port) {
            return GetSockAddr_in(address.AsInAddr(), port, (addr *)this->addr_.data());
        }
    };
};

} // namespace wutils::network::ip


#endif // UTIL_IP_H
