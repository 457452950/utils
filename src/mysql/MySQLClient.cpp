#include "wutils/experiment/mysql/MySQLClient.h"
#include <iostream>

namespace wutils {

MySQLClient::MySQLClient(/* args */) { mysql_init(&_connect); }

MySQLClient::~MySQLClient() {}

bool MySQLClient::Init(
        const char *addr, const char *user_name, const char *passwd, const char *db_name, uint16_t port) {
    if(addr == nullptr || user_name == nullptr || passwd == nullptr || db_name == nullptr) {
        return false;
    }

    if(mysql_real_connect(&_connect, addr, user_name, passwd, db_name, port, NULL, CLIENT_FOUND_ROWS)) {
    } else {
        return false;
    }
    return true;
}

void MySQLClient::Release() { mysql_close(&_connect); }


sqlRes_ptr MySQLClient::Query(const std::string &sql) {
    if(mysql_query(&this->_connect, sql.c_str())) {
        this->_errorMessage = std::string(mysql_error(&this->_connect));
        return nullptr;
    }
    return std::make_shared<MySQLResult>(mysql_store_result(&this->_connect));
}

uint64_t MySQLClient::EQuery(const std::string &sql) {
    if(mysql_query(&this->_connect, sql.c_str())) {
        this->_errorMessage = std::string(mysql_error(&this->_connect));
        return -1;
    }
    return mysql_affected_rows(&this->_connect);
}

} // namespace wutils
