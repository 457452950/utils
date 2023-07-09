#pragma once
#ifndef UTIL_TOOLS_H
#define UTIL_TOOLS_H


#include <cerrno>
#include <cstdint>
#include <fcntl.h> // fcntl
#include <regex>
#include <string>

#include <sys/socket.h>
#include <sys/timerfd.h>

#include <arpa/inet.h>   // inet_ntop inet_pton
#include <netinet/tcp.h> // tcp_nodelay

#include "wutils/SharedPtr.h"
#include "wutils/network/base/Defined.h"

namespace wutils::network {

inline bool HtoNS(uint16_t host_num, uint16_t *net_num) {
    *net_num = ::htons(host_num);
    return true;
}
inline bool NtoHS(uint16_t net_num, uint16_t *host_num) {
    *host_num = ::ntohs(net_num);
    return true;
}

namespace ip {
/***************************************************
 * Ip Utils
 ****************************************************/

inline bool IsValidIp4(const std::string &ip_str) {
    std::regex re("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25["
                  "0-5])$");
    return std::regex_match(ip_str, re);
}

inline bool IsValidIpv6(const std::string &ip_str) {
    std::regex re("");
    return std::regex_match(ip_str, re);
}

/**
 * @param ip_str
 * @param addr
 * @example
 *  ::in_addr addr{}; \n
 *  bool ok =ip::IpStrToAddr("127.0.0.1", &addr)
 */
inline bool IpStrToAddr(const std::string &ip_str, in_addr *addr) {
    if(::inet_pton(AF_INET, ip_str.c_str(), (void *)addr) == 1) {
        return true;
    }
    return false;
}

/**
 *
 * @param ip_str
 * @param addr
 * @example
 *  ::in6_addr addr{}; \n
 *  bool ok = ip::IpStrToAddr("::1", &addr);
 */
inline bool IpStrToAddr(const std::string &ip_str, in6_addr *addr) {
    if(::inet_pton(AF_INET6, ip_str.c_str(), (void *)addr) == 1) {
        return true;
    }
    return false;
}

/**
 * @param addr
 * @param buf
 * @example
 *  ::in_addr addr{}; \n
 *  std::string ip; \n
 *  bool ok = IpStrToAddr("127.0.0.1", &addr); \n
 *  ok = IpAddrToStr(addr, ip);
 */
inline bool IpAddrToStr(const in_addr &addr, std::string &buf) {
    constexpr int32_t  _len = 100;
    unique_ptr<char[]> _buf = make_unique<char[]>(_len);

    if(::inet_ntop((int)AF_FAMILY::INET, (void *)&addr, _buf.get(), _len) == nullptr) {
        buf.clear();
        return false;
    }

    buf.assign(_buf.get());
    return true;
}
/**
 *
 * @param addr
 * @param buf
 * @example
 *  ::in6_addr addr{}; \n
 *  std::string ip; \n
 *  bool ok = IpStrToAddr("::1", &addr); \n
 *  ok = IpAddrToStr(addr, ip);
 */
inline bool IpAddrToStr(const in6_addr &addr, std::string &buf) {
    constexpr int32_t  _len = 150;
    unique_ptr<char[]> _buf = make_unique<char[]>(_len);
    if(::inet_ntop((int)AF_FAMILY::INET6, (void *)&addr, _buf.get(), _len) == nullptr) {
        buf.clear();
        return false;
    }

    buf.assign(_buf.get());
    return true;
}

/**
 * @param in_addr
 * @param port
 * @param addr
 *
 * @example
 *  ::in_addr addr{}; \n
 *  sockaddr_in sockaddr{}; \n
 *  bool ok = IpStrToAddr("127.0.0.1", &addr);  \n
 *  ok = GetSockAddr_in(addr, 8000, &sockaddr);
 */
inline bool GetSockAddr_in(const ::in_addr &in_addr, uint16_t port, sockaddr_in *addr) {
    addr->sin_family = AF_INET;

    if(!HtoNS(port, &addr->sin_port)) {
        return false;
    }

    addr->sin_addr = in_addr;
    return true;
}
/**
 * @param in_addr
 * @param port
 * @param addr
 *
 * @example
 *  ::in6_addr addr{}; \n
 *  sockaddr_in6 sockaddr{}; \n
 *  bool ok = IpStrToAddr("::1", &addr); \n
 *  ok = GetSockAddr_in(addr, 8000, &sockaddr);
 */
inline bool GetSockAddr_in(const ::in6_addr &in_addr, uint16_t port, sockaddr_in6 *addr) {
    addr->sin6_family = AF_INET6;

    if(!HtoNS(port, &addr->sin6_port)) {
        return false;
    }

    addr->sin6_addr     = in_addr;
    addr->sin6_flowinfo = 0;
    return true;
}

/***************************************************
 * ISocket Utils
 ****************************************************/

// socket function
inline int SocketGetFlag(socket_t socket) { return ::fcntl(socket, F_GETFL, 0); }

inline bool SocketIsNonBlock(socket_t socket) { return SocketGetFlag(socket) & O_NONBLOCK; }

inline bool SetSocketNoBlock(socket_t socket, bool isSet) {
    if(isSet) {
        return ::fcntl(socket, F_SETFL, SocketGetFlag(socket) | O_NONBLOCK) != -1;
    }

    return ::fcntl(socket, F_SETFL, SocketGetFlag(socket) & (~O_NONBLOCK)) != -1;
}

inline bool SetSocketReuseAddr(socket_t socket, bool isSet) {
    int          opt = (int)isSet;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, len) == -1) {
        return false;
    }
    return true;
}
inline bool SetSocketReusePort(socket_t socket, bool isSet) {
    int          opt = (int)isSet;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &opt, len) == -1) {
        return false;
    }
    return true;
}
inline bool SetSocketKeepAlive(socket_t socket, bool isSet) {
    int          opt = (int)isSet;
    unsigned int len = sizeof(opt);
    if(::setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &opt, len) == -1) {
        return false;
    }
    return true;
}

// tcp socket function
inline bool SetTcpSocketNoDelay(socket_t socket, bool isSet) {
    int opt_val = (int)isSet;
    if(::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &opt_val, static_cast<socklen_t>(sizeof opt_val)) == 0) {
        return true;
    }
    return false;
}

} // namespace ip

namespace timer {
/**
 *
 * @return timer fd
 */
inline timer_t CreateNewTimerFd() {
    return ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    //    return ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
}
/**
 *  @enum
 *  REL 相对时间
 *  @enum
 *  ABS 绝对时刻
 */
enum class TimerFlag {
    /* 相对时间 */
    REL = 0,
    /* 绝对时刻 */
    ABS = 1,
};
/**
 *
 * @param fd  timer fd
 * @param flag  enum class TimerFlag
 * @param next_time next time to clock
 * @param prev_time if not null, repeat with this param
 * @return
 */
inline bool SetTimerTime(timer_t                    fd,
                         TimerFlag                  flag,
                         const struct ::itimerspec *next_time,
                         struct ::itimerspec       *prev_time = nullptr) {
    if(::timerfd_settime(fd, (int)flag, next_time, prev_time) == 0) {
        return true;
    }
    return false;
}

} // namespace timer

} // namespace wutils::network

#endif // UTIL_TOOLS_H
