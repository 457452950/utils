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
#include <arpa/inet.h>
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

bool Ip2String(in_addr addr, std::string& buf);
bool Ip2String(in6_addr addr, std::string& buf);


}   // namespace wlb
