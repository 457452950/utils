#pragma once

#include <string>
#include <stdint.h>
#include <exception>

#include <WOS.h>
#ifdef OS_IS_WINDOWS

#elif defined OS_IS_LINUX

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>    // tcp_nodelay
#include <arpa/inet.h>      // inet_ntop inet_pton
#include <cstring>
#include <cerrno>
#include <fcntl.h>

#endif

namespace wlb
{

enum class AF_FAMILY 
{
    INET = AF_INET,
    INET6 = AF_INET6
};

// sockaddr_in
// in_addr  in6_addr

bool IpAddrToString(in_addr addr, std::string& buf);
bool IpAddrToString(in6_addr addr, std::string& buf);

bool StringToIpAddress(std::string& ip_str, in_addr& addr);
bool StringToIpAddress(std::string& ip_str, in6_addr& addr);

}   // namespace wlb
