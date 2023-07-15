#include "wutils/network/Tools.h"

#include <cassert>
#include <iostream>

namespace wutils::network {

timerfd_t CreateNewTimerfd() {
    // return ::timerfd_create(CLOCK_REALTIME_ALARM, TFD_NONBLOCK | TFD_CLOEXEC);
    //    return ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    return ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    // return ::timerfd_create(CLOCK_REALTIME, 0);
}

bool SetTimerTime(timerfd_t fd, SetTimeFlag flag, const struct itimerspec *next_time, struct itimerspec *prev_time) {
    if(::timerfd_settime(fd, (int)flag, next_time, prev_time) == 0) {
        return true;
    }
    return false;
}



bool Bind(socket_t socket, const EndPoint &serverInfo) {
    auto ok = ::bind(socket, serverInfo.AsSockAddr(), serverInfo.GetSockSize());
    if(ok == 0) {
        return true;
    }

    return false;
}


socket_t MakeBindedSocket(const EndPoint &info) {
    socket_t bind_sock = INVALID_SOCKET;
    auto     fami      = info.GetFamily();

    bind_sock = MakeSocket(fami, AF_PROTOL::UDP);
    if(bind_sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    if(Bind(bind_sock, info)) {
        return bind_sock;
    }

    ::close(bind_sock);
    return INVALID_SOCKET;
}

socket_t MakeListenedSocket(const EndPoint &info) {
    socket_t listen_sock(INVALID_SOCKET);
    auto     fami = info.GetFamily();

    listen_sock = MakeSocket(fami, AF_PROTOL::TCP);
    if(listen_sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    if(Bind(listen_sock, info)) {
        if(::listen(listen_sock, 1024) == 0) {
            return listen_sock;
        }
    }

    ::close(listen_sock);
    return INVALID_SOCKET;
}

socket_t Accept(socket_t socket, EndPoint &info) {
    sockaddr_in6 temp{};
    socklen_t    len{sizeof(temp)};
    socket_t     _client_sock = ::accept(socket, (sockaddr *)&temp, &len);
    if(_client_sock < 0) {
        return INVALID_SOCKET;
    }
    // clang-format off
    info.Assign((sockaddr*)&temp, 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return _client_sock;
}

socket_t Accept4(socket_t socket, EndPoint &info, int flags) {
    sockaddr_in6 temp{};
    socklen_t    len{sizeof(temp)};
    socket_t     _client_sock = ::accept4(socket, (sockaddr *)&temp, &len, flags);
    if(_client_sock < 0) {
        return INVALID_SOCKET;
    }
    // clang-format off
    info.Assign((sockaddr*)&temp, 
        len == sizeof(sockaddr_in) ? 
        AF_FAMILY::INET : AF_FAMILY::INET6);
    // clang-format on

    return _client_sock;
}


socket_t ConnectToHost(const EndPoint &info, AF_PROTOL protol) {
    auto lis_sock = MakeSocket(info.GetFamily(), protol);
    if(lis_sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }
    return ConnectToHost(lis_sock, info);
}

bool ConnectToHost(socket_t socket, const EndPoint &info) {
    if(::connect(socket, info.AsSockAddr(), info.GetSockSize()) == 0) {
        return true;
    }

    return false;
}


int64_t RecvFrom(socket_t socket, uint8_t *buf, uint32_t buf_len, EndPoint &info) {
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

bool SetTcpSocketNoDelay(socket_t socket, bool is_set) {
    int opt = int(is_set);
    return ::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) == 0;
}


} // namespace wutils::network
