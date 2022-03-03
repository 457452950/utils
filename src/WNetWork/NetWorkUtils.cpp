#include "WNetWork/WNetWorkUtils.h"
#include <iostream>
#include "WDebugger.hpp"

namespace wlb::NetWork
{

using namespace wlb::debug;

#if OS_IS_WINDOWS



#elif OS_IS_LINUX

base_socket_type MakeTcpv4Socket()
{
    return ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

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


bool HtoNS(const uint16_t host_num, uint16_t& net_num)
{
    try
    {
        net_num = ::htons(host_num);
        return true;
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
    return false;
}
bool NtoHS(const uint16_t net_num, uint16_t& host_num)
{
    try
    {
        host_num = ::ntohs(net_num);
        return true;
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
    return false;
}

bool MakeSockAddr_in(const std::string& ip_address, uint16_t port, sockaddr_in& addr){
    addr.sin_family = AF_INET;
    uint16_t h_port = 0;
    in_addr h_addr;

    if (!HtoNS(port, h_port))
    {
        return false;
    }
    if (!StringToIpAddress(ip_address, h_addr))
    {
        return false;
    }
        
    addr.sin_port = h_port;
    addr.sin_addr = h_addr;
    return true;
}


bool Bind(base_socket_type socket, const std::string& host, uint16_t port, bool isv4)
{
    if (isv4)
    {    
        sockaddr_in ei{0};
        ei.sin_family   = AF_INET;

        in_addr ipaddr{0};
        if ( !StringToIpAddress(host, ipaddr))
        {
            return false;
        }
        
        if (!wlb::NetWork::HtoNS(port, ei.sin_port))
            return false;
        
        int32_t ok = ::bind(socket,
                    (struct sockaddr*)&(ei),
                    sizeof(ei));
        if ( ok == -1 )
        {
            return false;
        }
        
        return true;
    }
    return false;
}

bool Bind(base_socket_type socket, const WEndPointInfo& serverInfo)
{
    Bind(socket, serverInfo.ip_address, serverInfo.port, serverInfo.isv4);
}

base_socket_type Accept(base_socket_type socket, WEndPointInfo& info, bool isv4)
{
    if (isv4)
    {
        sockaddr_in client_info;
        socklen_t len = sizeof(client_info);
        base_socket_type clientsock = ::accept(
                                        socket,
                                        (struct sockaddr*)&client_info,
                                        &len);
        if (clientsock < 0)
        {
            return -1;
        }

        info = WEndPointInfo::FromNet(client_info);
        
        return clientsock;  
    }
    
}

base_socket_type AcceptV4(base_socket_type socket, WEndPointInfo& info) {
    return Accept(socket, info, true);
}

bool ConnectToHost(base_socket_type socket, const std::string& host, uint16_t port, bool isv4)
{
    if (isv4)
    {
        sockaddr_in addr;
        if (!MakeSockAddr_in(host, port, addr))
        {
            return false;
        }
        if (::connect(socket, (sockaddr*)&addr, sizeof(sockaddr_in)) == 0)
        {
            return true;
        }
    }
    else
    {
        return false;
    }
    return false;
}
bool ConnectToHost(base_socket_type socket, const WEndPointInfo& info)
{
    return ConnectToHost(socket, info.ip_address, info.port, info.isv4);
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
