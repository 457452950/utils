#pragma once
#ifndef UTILS_LIB_MYSQL_H
#define UTILS_LIB_MYSQL_H

#include <mysql/mysql.h>
#include <string>
#include <vector>

#include "wutils/SharedPtr.h"

namespace wutils {


class MySQLResult {
public:
    explicit MySQLResult(MYSQL_RES *res) : _res(res) {}
    ~MySQLResult() { mysql_free_result(this->_res); }

    bool GetNames(std::vector<std::string> &names) {
        names.clear();

        if(this->_res == nullptr) {
            return false;
        }

        auto         fields_num = mysql_num_fields(this->_res);
        MYSQL_FIELD *fields     = mysql_fetch_fields(this->_res);
        if(fields == nullptr) {
            return false;
        }

        for(int i = 0; i < fields_num; i++) {
            names.emplace_back(fields[i].name);
        }
        return true;
    }

    bool GetTableData(std::vector<std::vector<std::string>> &data) {
        data.clear();

        if(this->_res == nullptr) {
            return false;
        }

        auto fields_num = mysql_num_fields(this->_res);
        while(MYSQL_ROW row = mysql_fetch_row(this->_res)) {
            std::vector<std::string> _v1;
            for(size_t index = 0; index < fields_num; index++) {
                _v1.push_back(row[index]);
            }
            data.push_back(_v1);
        }
        return true;
    }
    bool GetTableData(std::vector<std::vector<std::string>> &data, uint32_t start, uint32_t count) {
        data.clear();

        if(this->_res == nullptr) {
            return false;
        }

        // 起始点非法
        if(start > this->GetRowCount()) {
            return false;
        }

        mysql_data_seek(this->_res, start);
        uint32_t has_count = 0; // 收录行数

        auto fields_num = mysql_num_fields(this->_res);
        while(MYSQL_ROW row = mysql_fetch_row(this->_res)) {
            if(has_count >= count) {
                break;
            }

            std::vector<std::string> _v1;
            for(size_t index = 0; index < fields_num; index++) {
                _v1.push_back(row[index]);
            }
            data.push_back(_v1);
            has_count++;
        }
        return true;
    }

    uint64_t GetRowCount() { return mysql_num_rows(this->_res); }

private:
    MYSQL_RES *_res;
};

using sqlRes_ptr = shared_ptr<MySQLResult>;

class MySQLClient {
public:
    MySQLClient(/* args */);
    ~MySQLClient();

    bool Init(const char *addr, const char *user_name, const char *passwd, const char *db_name, uint16_t port);
    void Release();

    sqlRes_ptr                Query(const std::string &sql);
    uint64_t                  EQuery(const std::string &sql);
    inline const std::string &GetErrorString() { return this->_errorMessage; }

private:
    MYSQL       _connect;
    std::string _errorMessage;
};

} // namespace wutils


#endif // UTILS_LIB_MYSQL_H
