#pragma once
#ifndef UTIL_ACCEPTOR_H
#define UTIL_ACCEPTOR_H

#include "wutils/network/Socket.h"
#include "wutils/network/Tools.h"

namespace wutils::network {

template <class FAMILY, class PROTOCOL, class SOCKET_T>
class IAcceptor : public ISocket {
public:
    IAcceptor() : ISocket(CreateSocket<FAMILY, PROTOCOL>()){};

    using Family      = FAMILY;
    using Protocol    = PROTOCOL;
    using Socket_type = SOCKET_T;

    void Assign(Socket_type socket, const typename FAMILY::EndPointInfo &info) {
        this->socket_ = socket;
        this->info_   = info;
    }

    Socket_type                   GetSocket() const { return socket_; }
    typename FAMILY::EndPointInfo GetLocalInfo() const { return info_; }

    bool Bind(const typename FAMILY::EndPointInfo &info) {
        this->info_ = info;
        return ::bind(this->socket_, (sockaddr *)info_.AsSockAddr(), info_.GetSockAddrLen()) == 0;
    }
    bool Listen() { return ::listen(this->socket_, 4096) == 0; }

    Socket_type Accept(typename FAMILY::EndPointInfo &info) {
        typename FAMILY::native_sockaddr_type _addr;
        socklen_t                             len = FAMILY::sockaddr_len;

        auto sock = ::accept(this->socket_, (sockaddr *)&_addr, &len);
        if(sock == -1) {
            return Socket_type(nullptr);
        }

        info.Assign(_addr);
        return Socket_type(sock);
    }

protected:
    typename FAMILY::EndPointInfo info_;
};

} // namespace wutils::network


#endif // UTIL_ACCEPTOR_H
