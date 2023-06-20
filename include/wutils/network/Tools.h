#pragma once
#ifndef UTIL_TOOLS_H
#define UTIL_TOOLS_H


#include <cerrno>
#include <cstdint>
#include <cstring> // strerror
#include <exception>
#include <fcntl.h> // fcntl
#include <memory>
#include <string>
#include <tuple>
#include <unistd.h>
#include <unordered_map> //std::hash

#include <sys/socket.h>
#include <sys/timerfd.h>

#include <arpa/inet.h>   // inet_ntop inet_pton
#include <netinet/tcp.h> // tcp_nodelay

#include "NetworkDef.h"

namespace wutils::network {

bool HtoNS(uint16_t host_num, uint16_t *net_num);
bool NtoHS(uint16_t net_num, uint16_t *host_num);

namespace ip {
/***************************************************
 * Ip Utils
 ****************************************************/
bool IpAddrToStr(const in_addr &addr, std::string &buf);
bool IpAddrToStr(const in6_addr &addr, std::string &buf);

bool IpStrToAddr(const std::string &ip_str, in_addr *addr);
bool IpStrToAddr(const std::string &ip_str, in6_addr *addr);

bool GetSockAddr_in(const ::in_addr &in_addr, uint16_t port, sockaddr_in *addr);
bool GetSockAddr_in(const ::in6_addr &in_addr, uint16_t port, sockaddr_in6 *addr);

/***************************************************
 * SOCKET Utils
 ****************************************************/

// socket function
bool SocketIsNoBlock(socket_t socket);
bool SetSocketNoBlock(socket_t socket, bool isSet);

bool SetSocketReuseAddr(socket_t socket, bool isSet);
bool SetSocketReusePort(socket_t socket, bool isSet);
bool SetSocketKeepAlive(socket_t socket, bool isSet);

// tcp socket function
bool SetTcpSocketNoDelay(socket_t socket, bool isSet);

} // namespace ip

namespace timer {

// 设置时间差的意义
enum class SetTimeFlag {
    REL = 0, // 相对时间
    ABS = 1, // 绝对时间
};

timer_t CreateNewTimerFd();
bool    SetTimerTime(timer_t                    fd,
                     SetTimeFlag                flag,
                     const struct ::itimerspec *next_time,
                     struct ::itimerspec       *prev_time = nullptr);

} // namespace timer

} // namespace wutils::network

#endif // UTIL_TOOLS_H
