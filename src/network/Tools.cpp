#include "wutils/network/Tools.h"

#include "wutils/SharedPtr.h"

#include <cassert>

namespace wutils::network {

namespace timer {

timer_t CreateNewTimerFd() {
    // return ::timerfd_create(CLOCK_REALTIME_ALARM, TFD_NONBLOCK | TFD_CLOEXEC);
    return ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    // return ::timerfd_create(CLOCK_REALTIME, 0);
}

bool SetTimerTime(timer_t fd, SetTimeFlag flag, const struct itimerspec *next_time, struct itimerspec *prev_time) {
    if(::timerfd_settime(fd, (int)flag, next_time, prev_time) == 0) {
        return true;
    }
    return false;
}
} // namespace timer

bool HtoNS(uint16_t host_num, uint16_t *net_num) {
    *net_num = ::htons(host_num);
    return true;
}
bool NtoHS(uint16_t net_num, uint16_t *host_num) {
    *host_num = ::ntohs(net_num);
    return true;
}

namespace ip {

bool IpAddrToStr(const in_addr &addr_, std::string &buf) {
    constexpr int32_t  _len = 100;
    unique_ptr<char[]> _buf = make_unique<char[]>(_len);

    if(::inet_ntop((int)AF_FAMILY::INET, (void *)&addr_, _buf.get(), _len) == nullptr) {
        buf.clear();
        return false;
    }

    buf.assign(_buf.get());
    return true;
}

bool IpAddrToStr(const in6_addr &addr, std::string &buf) {
    constexpr int32_t  _len = 150;
    unique_ptr<char[]> _buf = make_unique<char[]>(_len);
    if(::inet_ntop((int)AF_FAMILY::INET6, (void *)&addr, _buf.get(), _len) == nullptr) {
        buf.clear();
        return false;
    }

    buf.assign(_buf.get());
    return true;
}

bool IpStrToAddr(const std::string &ip_str, in_addr *addr) {
    if(::inet_pton(AF_INET, ip_str.c_str(), (void *)addr) == 1) {
        return true;
    }
    return false;
}

bool IpStrToAddr(const std::string &ip_str, in6_addr *addr) {
    if(::inet_pton(AF_INET6, ip_str.c_str(), (void *)addr) == 1) {
        return true;
    }
    return false;
}

bool ip::GetSockAddr_in(const ::in_addr &in_addr, uint16_t port, sockaddr_in *addr) {
    addr->sin_family = AF_INET;

    if(!HtoNS(port, &addr->sin_port)) {
        return false;
    }

    addr->sin_addr = in_addr;
    return true;
}

bool ip::GetSockAddr_in(const ::in6_addr &in_addr, uint16_t port, sockaddr_in6 *addr) {
    addr->sin6_family = AF_INET6;

    if(!HtoNS(port, &addr->sin6_port)) {
        return false;
    }

    addr->sin6_addr     = in_addr;
    addr->sin6_flowinfo = 0;
    return true;
}

/***************************************************
 * Socket Utils
 ****************************************************/
int SocketGetFlag(socket_t socket) { return ::fcntl(socket, F_GETFL, 0); }

bool SocketIsNoBlock(socket_t socket) { return SocketGetFlag(socket) & O_NONBLOCK; }

bool SetSocketNoBlock(socket_t socket, bool isSet) {
    if(isSet) {
        return ::fcntl(socket, F_SETFL, SocketGetFlag(socket) | O_NONBLOCK) != -1;
    }

    return ::fcntl(socket, F_SETFL, SocketGetFlag(socket) & (~O_NONBLOCK)) != -1;
}

bool SetSocketReuseAddr(socket_t socket, bool isSet) {
    int          opt = (int)isSet;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, len) == -1) {
        return false;
    }
    return true;
}
bool SetSocketReusePort(socket_t socket, bool isSet) {
    int          opt = (int)isSet;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &opt, len) == -1) {
        return false;
    }
    return true;
}
bool SetSocketKeepAlive(socket_t socket, bool isSet) {
    int          opt = (int)isSet;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &opt, len) == -1) {
        return false;
    }
    return true;
}

bool SetTcpSocketNoDelay(socket_t socket, bool isSet) {
    int opt_val = (int)isSet;
    if(::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &opt_val, static_cast<socklen_t>(sizeof opt_val)) == 0) {
        return true;
    }
    return false;
}
} // namespace ip

} // namespace wutils::network
