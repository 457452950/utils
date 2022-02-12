#pragma once

#include <string>
#include <stdint.h>
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

namespace wlb
{
namespace NetWork
{

using base_socket_type = int32_t;
using base_socket_ptr = base_socket_type*;

struct WPeerInfo
{
    std::string peer_address;
    uint16_t peer_port;
};

enum class AF_FAMILY 
{
    INET = AF_INET,
    INET6 = AF_INET6
};

// sockaddr_in
// in_addr  in6_addr

bool IpAddrToString(in_addr addr, std::string& buf);
bool IpAddrToString(in6_addr addr, std::string& buf);

bool StringToIpAddress(const std::string& ip_str, in_addr& addr);
bool StringToIpAddress(const std::string& ip_str, in6_addr& addr);

// socket function
bool SetSocketNoBlock(base_socket_type socket);
bool SetSocketReuseAddr(base_socket_type socket);
bool SetSocketReusePort(base_socket_type socket);
bool SetSocketKeepAlive(base_socket_type socket);


// tcp socket function
bool SetTcpSocketNoDelay(base_socket_type socket);

}   // namespace NetWork
}   // namespace wlb
