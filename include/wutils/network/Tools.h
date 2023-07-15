#pragma once
#ifndef UTILS_NETWORK_UTILS_H
#define UTILS_NETWORK_UTILS_H

#include <fcntl.h>  // fcntl
#include <string>
#include <unistd.h> // close(int)

#include <sys/timerfd.h>

#include <netinet/tcp.h> // tcp_nodelay

#include "wutils/base/HeadOnly.h"
#include "wutils/network/base/Native.h"
#include "EndPoint.h"

namespace wutils::network {


struct EndPoint;


// 设置时间差的意义
enum class SetTimeFlag {
    REL = 0, // 相对时间
    ABS = 1, // 绝对时间
};

timerfd_t CreateNewTimerfd();
bool      SetTimerTime(timerfd_t                fd,
                       SetTimeFlag              flag,
                       const struct itimerspec *next_time,
                       struct itimerspec       *prev_time = nullptr);


// sockaddr
// sockaddr_in      sockaddr_in6
// in_addr          in6_addr

// server methods
bool Bind(socket_t socket, const EndPoint &serverInfo);

socket_t MakeBindedSocket(const EndPoint &info);
socket_t MakeListenedSocket(const EndPoint &info);

/***************************************************
 * TCP Utils
 ****************************************************/
// return -1 if fail
socket_t Accept(socket_t socket, EndPoint &info);
socket_t Accept4(socket_t socket, EndPoint &info, int flags);

// tcp is default and return -1 if fail
socket_t ConnectToHost(const EndPoint &info, AF_PROTOL = AF_PROTOL::TCP);
bool     ConnectToHost(socket_t socket, const EndPoint &info);

/***************************************************
 * UDP Utils
 ****************************************************/

int64_t RecvFrom(socket_t socket, uint8_t *buf, uint32_t buf_len, EndPoint &info);


// socket function
HEAD_ONLY int GetSocketFlags(socket_t socket) { return ::fcntl(socket, F_GETFL, 0); }

HEAD_ONLY bool IsSocketNonBlock(socket_t socket) { return GetSocketFlags(socket) & O_NONBLOCK; }
HEAD_ONLY bool SetSocketNonBlock(socket_t socket, bool is_set) {
    if (is_set) {
        return ::fcntl(socket, F_SETFL, GetSocketFlags(socket) | O_NONBLOCK) == 0;
    } else {
        return ::fcntl(socket, F_SETFL, GetSocketFlags(socket) & (~O_NONBLOCK)) == 0;
    }
}
HEAD_ONLY bool SetSocketReuseAddr(socket_t socket, bool is_set) {
    int       opt = int(is_set);
    socklen_t len = sizeof(opt);
    return ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, len) == 0;
}
HEAD_ONLY bool SetSocketReusePort(socket_t socket, bool is_set) {
    int       opt = int(is_set);
    socklen_t len = sizeof(opt);
    return ::setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &opt, len) == 0;
}
HEAD_ONLY bool SetSocketKeepAlive(socket_t socket, bool is_set) {
    int       opt = int(is_set);
    socklen_t len = sizeof(opt);
    return ::setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &opt, len) == 0;
}

// tcp socket function
bool SetTcpSocketNoDelay(socket_t socket, bool is_set);
HEAD_ONLY bool SetTcpSocketKeepAlive(socket_t socket, bool is_set) {
    int flags = int(is_set);
    return setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags)) == 0;
}

//
HEAD_ONLY bool GetSockName(socket_t socket, EndPoint &info) {
    sockaddr_in6 sa{};
    socklen_t    len = sizeof (sockaddr_in6);

    auto ok = ::getsockname(socket, reinterpret_cast<sockaddr *>(&sa), &len) == 0;
    if(ok) {
        info.Assign((sockaddr *)&sa, len);
    }

    return ok;
}
HEAD_ONLY bool GetPeerName(socket_t socket, EndPoint &info) {
    v6::SockAddr sa{};
    socklen_t    len = v6::SOCKADDR_LEN;

    auto ok = ::getpeername(socket, reinterpret_cast<sockaddr *>(&sa), &len) == 0;
    if(ok) {
        info.Assign((sockaddr *)&sa, len);
    }

    return ok;
}


} // namespace wutils::network

#endif // UTILS_NETWORK_UTILS_H