#pragma once
#ifndef UTIL_IACCEPTOR_H
#define UTIL_IACCEPTOR_H

#include "ISocket.h"
#include "wutils/network/Tools.h"

namespace wutils::network {

template <class FAMILY, class PROTOCOL, class SOCKET_T>
class IAcceptor : public ISocket {
public:
    IAcceptor() : ISocket(CreateSocket<FAMILY, PROTOCOL>()){};

    using Family       = FAMILY;
    using Protocol     = PROTOCOL;
    using Socket_type  = SOCKET_T;
    using EndPointInfo = typename FAMILY::EndPointInfo;

    void Assign(ISocket socket, const EndPointInfo &info) {
        this->socket_ = socket;
        this->info_   = info;
    }

    socket_t     GetSocket() const { return socket_; }
    EndPointInfo GetLocalInfo() const { return info_; }

    bool Bind(const EndPointInfo &info) {
        this->info_ = info;
        return ::bind(this->socket_, (sockaddr *)info_.AsSockAddr(), info_.GetSockAddrLen()) == 0;
    }
    bool Listen() { return ::listen(this->socket_, 4096) == 0; }

    Socket_type Accept(EndPointInfo &info) {
        typename FAMILY::sockaddr_t _addr;
        socklen_t                   len = FAMILY::sockaddr_len;

        auto sock = ::accept(this->socket_, (sockaddr *)&_addr, &len);
        if(sock == INVALID_SOCKET) {
            return Socket_type(nullptr);
        }

        info.Assign(_addr);
        return Socket_type(sock);
    }

protected:
    EndPointInfo info_;
};

} // namespace wutils::network


#endif // UTIL_IACCEPTOR_H
