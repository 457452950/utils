#pragma once
#ifndef UTIL_TCP_H
#define UTIL_TCP_H

#include "Tools.h"
#include "wutils/network/NetAddress.h"
#include "wutils/network/base/Definition.h"
#include "wutils/network/base/ISocket.h"

namespace wutils::network::tcp {

enum SHUT_DOWN : int8_t { READ = SHUT_RD, WRITE = SHUT_WR, RDWR = SHUT_RDWR };

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
            this->socket_ = MakeTcpV4Socket();
            break;
        case INET6:
            this->socket_ = MakeTcpV6Socket();
            break;
        case UNIX:
        default:
            abort();
            return false;
        }
        return this->socket_ != INVALID_SOCKET;
    }

    int64_t SendSome(const uint8_t *data, uint32_t len) { return ::send(this->socket_, data, len, 0); }

    /**
     * will block until fully send or send failed
     * @return -1 for failed, number for sent
     */
    int64_t Send(const uint8_t *data, uint32_t len) {
        int64_t total = 0;
        while(len) {
            auto l = this->SendSome(data, len);
            if(l == -1) {
                if(errno == EAGAIN) {
                    continue;
                }
                return -1;
            }
            total += l;

            len  -= l;
            data += l;
        }
        return total;
    }

    int64_t Recv(uint8_t *buffer, uint32_t len) { return recv(this->socket_, buffer, len, 0); }

    void ShutDown(SHUT_DOWN how) { ::shutdown(this->socket_, how); }

    NetAddress GetLocal() {
        NetAddress info;

        GetSockName(this->socket_, info);

        return info;
    }

    NetAddress GetRemote() {
        NetAddress info;

        GetPeerName(this->socket_, info);

        return info;
    }

    // Server
    bool Bind(const NetAddress &local) { return ::bind(this->socket_, local.AsSockAddr(), local.GetSockSize()) == 0; }

    bool Listen() { return ::listen(this->socket_, MAX_LISTEN_BACK_LOG) == 0; }

    ISocket Accept(NetAddress &info, bool set_nonblock = false) {
        if(set_nonblock) {
            return network::Accept4(this->socket_, info, SOCK_NONBLOCK);
        }
        return network::Accept(this->socket_, info);
    }

    // Client
    bool Connect(const NetAddress &remote) { return ConnectToHost(this->socket_, remote); }

    // Socket optional
    bool IsNonBlock() { return IsSocketNonBlock(this->socket_); }
    bool SetNonBlock(bool is_set) { return SetSocketNonBlock(this->socket_, is_set); }
    bool SetPortReuse(bool is_set) { return SetSocketReusePort(this->socket_, is_set); }
    bool SetAddrReuse(bool is_set) { return SetSocketReuseAddr(this->socket_, is_set); }
    bool SetNoDelay(bool is_set) { return SetTcpSocketNoDelay(this->socket_, is_set); }
    bool SetKeepAlive(bool is_set) { return SetSocketKeepAlive(this->socket_, is_set); }
};

} // namespace wutils::network::tcp


#endif // UTIL_TCP_H
