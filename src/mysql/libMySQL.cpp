#include "libMySQL.h"
#include "Logger.h"
#include <iostream>

namespace wlb
{
    using namespace Log;

    libMySQL::libMySQL(/* args */)
    {
        mysql_init(&_connect);
    }

    libMySQL::~libMySQL()
    {
    }

    bool libMySQL::Init(const char* addr, const char* user_name, const char* passwd, const char* db_name, uint16_t port)
    {
        if (addr == nullptr || user_name == nullptr || passwd == nullptr || db_name == nullptr)
        {
            return false;
        }
        std::cout << " ip:" << addr
                        << "port:" << port
                        << "user_name" << user_name
                        << "db_name" << db_name << std::endl;
        if (mysql_real_connect(&_connect, addr, user_name, passwd, db_name, port, NULL, CLIENT_FOUND_ROWS))
        {
        }
        else
        {
            return false;
        }
        return true;
    }
    
    void libMySQL::Release()
    {
        mysql_close(&_connect);
    }


    sqlRes_ptr libMySQL::Query(const std::string& sql)
    {
        if (mysql_query(&this->_connect, sql.c_str()))
        {
            this->_errorMessage = std::string(mysql_error(&this->_connect));
            return nullptr;
        }
        return std::make_shared<libMySQLResult>(mysql_store_result(&this->_connect));
        
    }

    int64_t libMySQL::EQuery(const std::string& sql)
    {
        if (mysql_query(&this->_connect, sql.c_str()))
        {
            this->_errorMessage = std::string(mysql_error(&this->_connect));
            return -1;
        }
        return mysql_affected_rows(&this->_connect);
    }

} // namespace wlb

