#pragma once
#ifndef UTILS_NETWORK_UTILS_H
#define UTILS_NETWORK_UTILS_H

#include <fcntl.h>  // fcntl
#include <unistd.h> // close

#include <sys/timerfd.h> // timerfd_settime

#include <netinet/tcp.h> // tcp_nodelay

#include "wutils/base/HeadOnly.h"
#include "wutils/network/base/Native.h"
#include "wutils/network/NetAddress.h"

namespace wutils::network {


HEAD_ONLY timerfd_t CreateNewTimerfd() {
    // return ::timerfd_create(CLOCK_REALTIME_ALARM, TFD_NONBLOCK | TFD_CLOEXEC);
    //    return ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    return ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    // return ::timerfd_create(CLOCK_REALTIME, 0);
}

// 设置时间差的意义
enum class SetTimeFlag {
    REL = 0, // 相对时间
    ABS = 1, // 绝对时间
};
HEAD_ONLY bool SetTimerTimeOut(timerfd_t                fd,
                               SetTimeFlag              flag,
                               const struct itimerspec *next_time,
                               struct itimerspec       *last_time = nullptr) {
    if(::timerfd_settime(fd, (int)flag, next_time, last_time) == 0) {
        return true;
    }
    return false;
}

// server methods
HEAD_ONLY bool Bind(socket_t socket, const NetAddress &serverInfo) {
    auto ok = ::bind(socket, serverInfo.AsSockAddr(), serverInfo.GetSockSize());
    if(ok == 0) {
        return true;
    }

    return false;
}

/***************************************************
 * TCP Utils
 ****************************************************/
// return -1 if fail
HEAD_ONLY socket_t Accept(socket_t socket, NetAddress *info) {
    sockaddr_in6 temp{};
    socklen_t    len{sizeof(temp)};

    socket_t _client_sock = ::accept(socket, (sockaddr *)&temp, &len);
    if(_client_sock < 0) {
        return INVALID_SOCKET;
    }

    if(info) {
        info->Assign((sockaddr *)&temp, len);
    }

    return _client_sock;
}
// return -1 if fail
HEAD_ONLY socket_t Accept4(socket_t socket, NetAddress *info, int flags) {
    sockaddr_in6 temp{};
    socklen_t    len{sizeof(temp)};

    socket_t _client_sock = ::accept4(socket, (sockaddr *)&temp, &len, flags);
    if(_client_sock < 0) {
        return INVALID_SOCKET;
    }
    
    if(info) {
        info->Assign((sockaddr *)&temp, len);
    }

    return _client_sock;
}

// return INVALID_SOCKET if fail
HEAD_ONLY bool ConnectToHost(socket_t socket, const NetAddress &info) {
    if(::connect(socket, info.AsSockAddr(), info.GetSockSize()) == 0) {
        return true;
    }

    return false;
}

/***************************************************
 * UDP Utils
 ****************************************************/

HEAD_ONLY int64_t RecvFrom(socket_t socket, uint8_t *buf, uint32_t buf_len, NetAddress &info) {
    socklen_t    len = 0;
    sockaddr_in6 temp{0};

    auto recv_len = ::recvfrom(socket, buf, buf_len, 0, (sockaddr *)&temp, &len);
    if(recv_len < 0) {
        return -1;
    }

    // clang-format off
    info.Assign((sockaddr*)&temp,
        len == sizeof(sockaddr_in) ?
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return recv_len;
}


// socket function
HEAD_ONLY int GetSocketFlags(socket_t socket) { return ::fcntl(socket, F_GETFL, 0); }

HEAD_ONLY bool IsSocketNonBlock(socket_t socket) { return GetSocketFlags(socket) & O_NONBLOCK; }
HEAD_ONLY bool SetSocketNonBlock(socket_t socket, bool is_set) {
    if(is_set) {
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
HEAD_ONLY bool SetTcpSocketNoDelay(socket_t socket, bool is_set) {
    int opt = int(is_set);
    return ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == 0;
}
HEAD_ONLY bool SetTcpSocketKeepAlive(socket_t socket, bool is_set) {
    int flags = int(is_set);
    return setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags)) == 0;
}

//
HEAD_ONLY bool GetSockName(socket_t socket, NetAddress &info) {
    sockaddr_in6 sa{};
    socklen_t    len = sizeof(sockaddr_in6);

    auto ok = ::getsockname(socket, reinterpret_cast<sockaddr *>(&sa), &len) == 0;
    if(ok) {
        info.Assign((sockaddr *)&sa, len);
    }

    return ok;
}
HEAD_ONLY bool GetPeerName(socket_t socket, NetAddress &info) {
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