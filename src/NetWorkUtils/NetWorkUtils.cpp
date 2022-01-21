#include "../../include/WNetWorkUtils.h"
#include <iostream>

namespace wlb
{

bool IpAddrToString(in_addr addr, std::string& buf)
{
    try
    {
        int32_t _len = 30;
        char* _buf = new char[_len];
        if ( ::inet_ntop((int)AF_FAMILY::INET, (void*)&addr, _buf, _len) == nullptr)
        {
            buf.clear();
            return false;
        }

        buf.assign(_buf);
        delete[] _buf;
        return true;
    }
    catch(const std::exception& e)
    {
        std::cout << "StringToIpAddress : " << e.what() << " : " << strerror(errno) << std::endl;
    }
    return false;
}

bool IpAddrToString(in6_addr addr, std::string& buf)
{
    try
    {
        int32_t _len = 50;
        char* _buf = new char[_len];
        if ( ::inet_ntop((int)AF_FAMILY::INET6, (void*)&addr, _buf, _len) == nullptr)
        {
            buf.clear();
            return false;
        }

        buf.assign(_buf);
        delete[] _buf;
        return true;
    }
    catch(const std::exception& e)
    {
        std::cout << "StringToIpAddress : " << e.what() << " : " << strerror(errno) << std::endl;
    }
    return false;
}


bool StringToIpAddress(std::string& ip_str, in_addr& addr)
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
        std::cout << "StringToIpAddress : " << e.what() << " : " << strerror(errno) << std::endl;
    }
    return false;
}


bool StringToIpAddress(std::string& ip_str, in6_addr& addr)
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
        std::cout << "StringToIpAddress : " << e.what() << " : " << strerror(errno) << std::endl;
    }
    return false;
}

}   // namespace wlb
