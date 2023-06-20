#pragma once
#ifndef UTIL_ACCEPTOR_H
#define UTIL_ACCEPTOR_H

#include "wutils/network/Ip.h"

namespace wutils::network::ip {

template <class FAMILY, class PROTOCOL>
class ACCEPTOR {
public:
    ACCEPTOR() = default;

    void Assign(typename PROTOCOL::Socket socket, const typename FAMILY::EndPointInfo &info) {
        this->socket_ = socket;
        this->info_   = info;
    }
    typename PROTOCOL::Socket     GetSocket() const { return socket_; }
    typename FAMILY::EndPointInfo GetLocalInfo() const { return info_; }

    bool Bind(const typename FAMILY::EndPointInfo &info) {
        this->info_ = info;
        return ::bind(this->socket_.GetNativeSocket(), info_.AsSockAddr(), info_.GetSockAddrLen()) == 0;
    }
    bool Listen() { return ::listen(this->socket_.GetNativeSocket(), 4096) == 0; }

    typename PROTOCOL::Socket Accept(typename FAMILY::EndPointInfo &info) {
        typename FAMILY::addr _addr;
        socklen_t             len;

        auto sock = ::accept(this->socket_->GetNativeSocket(), (sockaddr *)&_addr, &len);
        if(sock == -1) {
            return PROTOCOL::Socket(nullptr);
        }

        info.Assign(_addr);
        return PROTOCOL::Socket(sock);
    }

protected:
    typename PROTOCOL::Socket     socket_;
    typename FAMILY::EndPointInfo info_;
};

} // namespace wutils::network::ip


#endif // UTIL_ACCEPTOR_H
