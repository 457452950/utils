#pragma once
#ifndef UTIL_TCP_H
#define UTIL_TCP_H


#include "Channel.h"
#include "Socket.h"
#include "Tools.h"
#include "base/Acceptor.h"
#include "base/EndPoint.h"

namespace wutils::network::ip {

class Tcp {
public:
    static int Protocol() { return IPPROTO_TCP; }
    static int Type() { return SOCK_STREAM; }

    template <class FAMILY>
    class Socket : public SOCKET {
    public:
        Socket() : SOCKET(CreateSocket<FAMILY, Tcp>()){};
        Socket(const Socket &other) : SOCKET(other) {}
        ~Socket() override = default;

        Socket &operator=(const Socket &);

        bool Bind(const typename FAMILY::EndPointInfo &info) {
            return ::bind(this->socket_.GetNativeSocket(), info.AsSockAddr(), info.GetSockAddrLen()) == 0;
        }

        int64_t Recv(const uint8_t *buffer, uint32_t buffer_len);
        int64_t Send(const uint8_t *data, uint32_t data_len);
    };

    template <class FAMILY>
    class Acceptor : public ACCEPTOR<FAMILY, Tcp> {};

    template <class FAMILY>
    class AcceptorCh : public AcceptorChannel<FAMILY, Tcp> {};
};

template <class FAMILY>
Tcp::Socket<FAMILY> &Tcp::Socket<FAMILY>::operator=(const Tcp::Socket<FAMILY> &other) {
    SOCKET::operator=(other);

    return *this;
}

template <class FAMILY>
int64_t Tcp::Socket<FAMILY>::Recv(const uint8_t *buffer, uint32_t buffer_len) {
    return ::recv(this->socket_, buffer, buffer_len, 0);
}
template <class FAMILY>
int64_t Tcp::Socket<FAMILY>::Send(const uint8_t *data, uint32_t data_len) {
    return ::send(this->socket_, data, data_len, 0);
}


} // namespace wutils::network::ip

#endif // UTIL_TCP_H
