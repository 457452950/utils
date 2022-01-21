#include "../../include/WNetWorkUtils.h"

namespace wlb
{

bool Ip2String(in_addr addr, std::string& buf)
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
        return true;
    }
    catch(const std::exception& e)
    {
        e.what();
    }
    return false;
}

bool Ip2String(in6_addr addr, std::string& buf)
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
        return true;
    }
    catch(const std::exception& e)
    {
        e.what();
    }
    return false;
}

}   // namespace wlb
