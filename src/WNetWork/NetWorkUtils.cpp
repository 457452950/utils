#include "WNetWork/WNetWorkUtils.h"
#include <iostream>
#include "WDebugger.hpp"

namespace wlb::NetWork
{

using namespace wlb::debug;

#if OS_IS_WINDOWS



#elif OS_IS_LINUX

bool IpAddrToString(in_addr addr, std::string& buf)
{
    try
    {
        int32_t _len = 30;
        char* _buf = new char[_len];
        NEWADD;
        if ( ::inet_ntop((int)AF_FAMILY::INET, (void*)&addr, _buf, _len) == nullptr)
        {
            buf.clear();
            return false;
        }

        buf.assign(_buf);
        delete[] _buf;
        DELADD;
        return true;
    }
    catch(const std::exception& e)
    {
        
    }
    return false;
}

bool IpAddrToString(in6_addr addr, std::string& buf)
{
    try
    {
        int32_t _len = 50;
        char* _buf = new char[_len];
        NEWADD;
        if ( ::inet_ntop((int)AF_FAMILY::INET6, (void*)&addr, _buf, _len) == nullptr)
        {
            buf.clear();
            return false;
        }

        buf.assign(_buf);
        delete[] _buf;
        DELADD;
        return true;
    }
    catch(const std::exception& e)
    {
        
    }
    return false;
}


bool StringToIpAddress(const std::string& ip_str, in_addr& addr)
{
    try
    {
        if ( ::inet_pton(AF_INET, ip_str.c_str(), (void*)&addr) == 1)
        {
            return true;
        }
    }
    catch(const std::exception& e)
    {
        
    }
    return false;
}


bool StringToIpAddress(const std::string& ip_str, in6_addr& addr)
{
    try
    {
        if ( ::inet_pton(AF_INET6, ip_str.c_str(), (void*)&addr) == 1)
        {
            return true;
        }
    }
    catch(const std::exception& e)
    {
        
    }
    return false;
}

bool SetSocketNoBlock(base_socket_type socket)
{
    if ( ::fcntl(socket, F_SETFL, ::fcntl(socket, F_GETFL, 0) | O_NONBLOCK) == -1 )
    {
        return false;
    }
    return true;
}

bool SetSocketReuseAddr(base_socket_type socket)
{
    try
    {
        int opt = 1;
        unsigned int len = sizeof(opt);
        if ( ::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, len) == -1 )
        {
            return false;
        }
        return true;
    }
    catch(const std::exception& e)
    {
        
    }
    return false;
}
bool SetSocketReusePort(base_socket_type socket)
{
    try
    {
        int opt = 1;
        unsigned int len = sizeof(opt);
        if ( ::setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, &opt, len) == -1 )
        {
            return false;
        }
        return true;
    }
    catch(const std::exception& e)
    {
        
    }
    return false;
}
bool SetSocketKeepAlive(base_socket_type socket)
{
    try
    {
        int opt = 1;
        unsigned int len = sizeof(opt);
        if ( ::setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &opt, len) == -1 )
        {
            return false;
        }
        return true;
    }
    catch(const std::exception& e)
    {
        
    }
    return false;
}

bool SetTcpSocketNoDelay(base_socket_type socket)
{
    int optval = 1;
    if (::setsockopt(socket, 
                    IPPROTO_TCP, 
                    TCP_NODELAY,
                    &optval, 
                    static_cast<socklen_t>(sizeof optval)
                    ) == 0)
    {
        return true;
    } 
    return false;
}



#endif // OS_IS_LINUX

}   // namespace wlb::NetWork
