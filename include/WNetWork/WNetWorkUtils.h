#pragma once

#include <string>
#include <cstdint>
#include <exception>

#include "../WOS.h"
#ifdef OS_IS_WINDOWS

#elif defined OS_IS_LINUX

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>    // tcp_nodelay
#include <arpa/inet.h>      // inet_ntop inet_pton
#include <cstring>          // strerror
#include <cerrno>
#include <fcntl.h>          // fcntl

#endif

namespace wlb::NetWork {

using base_socket_type = int32_t;
using base_socket_ptr = base_socket_type *;

enum class AF_FAMILY {
    INET  = AF_INET,
    INET6 = AF_INET6
};

struct WEndPointInfo;

// sockaddr
// sockaddr_in sockaddr_in6

// in_addr  in6_addr

base_socket_type MakeTcpV4Socket();

bool IpAddrToString(in_addr addr_, std::string *buf);
bool IpAddrToString(in6_addr addr, std::string *buf);

bool StringToIpAddress(const std::string &ip_str, in_addr *addr);
bool StringToIpAddress(const std::string &ip_str, in6_addr *addr);

bool HtoNS(uint16_t host_num, uint16_t *net_num);
bool NtoHS(uint16_t net_num, uint16_t *host_num);

bool MakeSockAddr_in(const std::string &ip_address, uint16_t port, sockaddr_in *addr);
// bool MakeSockAddr_in6(const std::string& ip_address, uint16_t port, sockaddr_in6* addr);

// server methods
bool Bind(base_socket_type socket, const std::string &host, uint16_t port, bool isv4 = true);
bool Bind(base_socket_type socket, const WEndPointInfo &serverInfo);

base_socket_type Accept(base_socket_type socket, WEndPointInfo *info, bool isv4 = true);
base_socket_type AcceptV4(base_socket_type socket, WEndPointInfo *info);

bool ConnectToHost(base_socket_type socket, const std::string &host, uint16_t port, bool isv4 = true);
bool ConnectToHost(base_socket_type socket, const WEndPointInfo &info);

// socket function
bool SetSocketNoBlock(base_socket_type socket);
bool SetSocketReuseAddr(base_socket_type socket);
bool SetSocketReusePort(base_socket_type socket);
bool SetSocketKeepAlive(base_socket_type socket);

// tcp socket function
bool SetTcpSocketNoDelay(base_socket_type socket);

// IP + port + isv4
struct WEndPointInfo {
    std::string ip_address;
    uint16_t    port;
    bool        isv4;
    WEndPointInfo(const std::string &_address = "", uint16_t _port = 0, bool _isv4 = true);;
    static WEndPointInfo FromNet(const sockaddr_in &net);
    // static const WEndPointInfo FromNet(const sockaddr_in6& net) {
    //     WEndPointInfo info;
    //     return info;
    // }
    bool ToNet4(sockaddr_in *_sockaddr_in);
};

}   // namespace wlb
