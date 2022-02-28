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
            LOG(L_ERROR) << "cant be null";
            return false;
        }
        std::cout << " ip:" << addr
                        << "port:" << port
                        << "user_name" << user_name
                        << "db_name" << db_name << std::endl;
        if (mysql_real_connect(&_connect, addr, user_name, passwd, db_name, port, NULL, CLIENT_FOUND_ROWS))
        {
            LOG(L_INFO) << "连接数据库成功 ip:" << addr
                        << "port:" << port
                        << "user_name" << user_name
                        << "db_name" << db_name;
        }
        else
        {
            LOG(L_ERROR) << "连接数据库失败 ip:" << addr
                        << "port:" << port
                        << "user_name" << user_name
                        << "db_name" << db_name;
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

} // namespace wlb

