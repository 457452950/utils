#pragma once
#ifndef UTIL_UDP_H
#define UTIL_UDP_H


#include "wutils/network/IO_Event.h"
#include "wutils/network/NetWork.h"
#include "wutils/network/Tools.h"
#include "wutils/network/base/Defined.h"
#include "wutils/network/base/EndPoint.h"
#include "wutils/network/base/IAcceptor.h"
#include "wutils/network/base/ISocket.h"

namespace wutils::network::ip {

class udp {
public:
    static int Protocol() { return IPPROTO_UDP; }
    static int Type() { return SOCK_DGRAM; }

    /**
     * @tparam FAMILY IP版本 V4 or V6
     * @example
     *  Socket<V4> s1;
     * @example
     *  Socket<V6> s2;
     */
    template <class FAMILY>
    class Socket : public ISocket {
    public:
        Socket() : ISocket(CreateSocket<FAMILY, udp>()){};
        Socket(const Socket &other) : ISocket(other) {}
        ~Socket() = default;

        using Family       = FAMILY;
        using EndPointInfo = typename Family::EndPointInfo;

        Socket &operator=(const Socket &other) {
            ISocket::operator=(other);
            return *this;
        }

        bool Bind(const EndPointInfo &info) {
            return ::bind(this->socket_.GetNativeSocket(), info.AsSockAddr(), info.GetSockAddrLen()) == 0;
        }

        // TODO: listen, connect, shutdown, accept

        int64_t Recv(const uint8_t *buffer, uint32_t buffer_len) {
            return ::recv(this->socket_, buffer, buffer_len, 0);
        }
        int64_t Send(const uint8_t *data, uint32_t data_len) { return ::send(this->socket_, data, data_len, 0); }

        int64_t RecvFrom(const uint8_t *buffer, uint32_t buffer_len, typename FAMILY::EndPointInfo &info) {
            typename FAMILY::native_sockaddr_type _sockaddr;
            socklen_t                             l;
            auto len = ::recvfrom(this->socket_, buffer, buffer_len, 0, &_sockaddr, &l);

            if(len > 0)
                info.Assign(_sockaddr);
            return len;
        }
        int64_t SendTo(const uint8_t *data, uint32_t data_len, EndPointInfo &info) {
            return ::sendto(this->socket_, data, data_len, 0, info.AsSockAddr(), info.GetSockAddrLen());
        }
    };
};


} // namespace wutils::network::ip


#endif // UTIL_UDP_H
