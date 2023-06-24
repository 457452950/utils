#pragma once
#ifndef UTIL_TCP_H
#define UTIL_TCP_H


// #include "Channel.h"
#include "Socket.h"
#include "Tools.h"
#include "base/Acceptor.h"
#include "base/EndPoint.h"

namespace wutils::network::ip {

class tcp {
public:
    static int Protocol() { return IPPROTO_TCP; }
    static int Type() { return SOCK_STREAM; }

    enum ShutType { SHUT_RD, SHUT_WR, SHUT_RDWR };
    /**
     *
     * @tparam FAMILY ipv4 or ipv6
     */
    template <class FAMILY>
    class Socket : public ISocket {
    public:
        Socket() : ISocket(CreateSocket<FAMILY, tcp>()){};
        explicit Socket(std::nullptr_t p) : ISocket(p) {}
        explicit Socket(socket_t s) : ISocket(s) {}
        Socket(const Socket &other) : ISocket(other) {}
        ~Socket() = default;

        Socket &operator=(const Socket &other) {
            ISocket::operator=(other);
            return *this;
        }

        bool Bind(const typename FAMILY::EndPointInfo &info) {
            return ::bind(this->socket_, (sockaddr *)info.AsSockAddr(), info.GetSockAddrLen()) == 0;
        }

        // TODO: connect, shutdown

        int64_t Recv(uint8_t *buffer, uint32_t buffer_len) { return ::recv(this->socket_, buffer, buffer_len, 0); }
        int64_t Send(const uint8_t *data, uint32_t data_len) { return ::send(this->socket_, data, data_len, 0); }
        bool    Connect(const typename FAMILY::EndPointInfo &info) {
            return ::connect(this->socket_, (sockaddr *)info.AsSockAddr(), info.GetSockAddrLen()) == 0;
        }
        void ShutDown(ShutType how) { ::shutdown(this->socket_, how); }
    };

    template <class FAMILY>
    class Acceptor : public IAcceptor<FAMILY, tcp, tcp::Socket<FAMILY>> {
    public:
        Acceptor() : IAcceptor<FAMILY, tcp, tcp::Socket<FAMILY>>(){};

        bool Bind(const typename FAMILY::EndPointInfo &info) {
            SetSocketReuseAddr(this->socket_, true);
            return IAcceptor<FAMILY, tcp, tcp::Socket<FAMILY>>::Bind(info);
        }

        bool IsNonBlock() { return SocketIsNonBlock(this->socket_); }
        bool SetNonBlock() { return SetSocketNoBlock(this->socket_, true); }
        bool SetReusePort() { return SetSocketReusePort(this->socket_, true); }
    };

    //    template <class FAMILY>
    //    class AcceptorCh : public AcceptorChannel<FAMILY, tcp> {};

    // just use like a namespace
private:
    tcp()  = default;
    ~tcp() = default;
};

} // namespace wutils::network::ip

#endif // UTIL_TCP_H
