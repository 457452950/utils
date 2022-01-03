#pragma once
#include <string>
#include <mysql/mysql.h>

namespace wlb
{
class libMySQL
{
private:
    /* data */
public:
    libMySQL(/* args */);
    ~libMySQL();

    bool Init(const char* addr, const char* user_name, const char* passwd, const char* db_name, uint16_t port);
    void Release();

private:
    MYSQL _connect;

};


    
} // namespace wlb



