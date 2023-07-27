#pragma once
#ifndef UTIL_UDP_H
#define UTIL_UDP_H

#include "Tools.h"
#include "wutils/network/NetAddress.h"
#include "wutils/network/base/Definition.h"
#include "wutils/network/base/ISocket.h"

namespace wutils::network::udp {

class Socket : public ISocket {
public:
    Socket()  = default;
    ~Socket() = default;

    Socket(const Socket &other) = default;
    explicit Socket(const ISocket &other) { this->socket_ = other.Get(); };

    using ISocket::operator=;
    Socket &operator=(const Socket &other) = default;

    using ISocket::operator bool;

    // Common
    bool Open(AF_FAMILY family) {
        switch(family) {
        case INET:
            this->socket_ = MakeUdpV4Socket();
            break;
        case INET6:
            this->socket_ = MakeUdpV6Socket();
            break;
        case UNIX:
        default:
            abort();
            return false;
        }
        return this->socket_ != INVALID_SOCKET;
    }
    int64_t SendTo(const uint8_t *data, uint32_t len, const NetAddress &remote) {
        assert(len < MAX_WAN_UDP_MSS);
        return ::sendto(this->socket_, data, len, 0, remote.AsSockAddr(), remote.GetSockSize());
    }
    int64_t RecvFrom(uint8_t *buffer, uint32_t len, NetAddress *remote) {
        sockaddr_in6 sa{};
        socklen_t    salen{sizeof(sa)};

        int64_t rlen = ::recvfrom(this->socket_, buffer, len, 0, (sockaddr *)&sa, &salen);
        if(remote) {
            remote->Assign((sockaddr *)&sa, salen);
        }
        return rlen;
    }
    int64_t Send(const uint8_t *data, uint32_t len) {
        assert(len < MAX_WAN_UDP_MSS);
        return ::send(this->socket_, data, len, 0);
    }
    int64_t Recv(uint8_t *buffer, uint32_t len) { return recv(this->socket_, buffer, len, 0); }

    NetAddress GetLocal() {
        NetAddress info;

        GetSockName(this->socket_, info);

        return info;
    }

    // Server
    bool Bind(const NetAddress &local) { return ::bind(this->socket_, local.AsSockAddr(), local.GetSockSize()) == 0; }

    // Client
    bool Connect(const NetAddress &remote) { return ConnectToHost(this->socket_, remote); }

    // Socket optional
    bool IsNonBlock() { return IsSocketNonBlock(this->socket_); }
    bool SetNonBlock(bool is_set) { return SetSocketNonBlock(this->socket_, is_set); }
    bool SetPortReuse(bool is_set) { return SetSocketReusePort(this->socket_, is_set); }
    bool SetAddrReuse(bool is_set) { return SetSocketReuseAddr(this->socket_, is_set); }
};


} // namespace wutils::network::udp


#endif // UTIL_UDP_H
