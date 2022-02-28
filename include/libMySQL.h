#pragma once
#include <string>
#include <memory>
#include <vector>
#include <mysql/mysql.h>

namespace wlb
{


class libMySQLResult
{
public:
    libMySQLResult(MYSQL_RES * res){ this->_res = res; }
    ~libMySQLResult() {mysql_free_result(this->_res);}

    bool GetNames(std::vector<std::string>& names){
        names.clear();

        if (this->_res == nullptr)
        {
            return false;
        }
        
        auto fields_num = mysql_num_fields(this->_res);
        MYSQL_FIELD* fields = mysql_fetch_fields(this->_res);
        if (fields == nullptr)
        {
            return false;
        }
        
        for (int i = 0; i < fields_num; i++)
        {
            names.push_back(fields[i].name);
        }
        return true;
    }

    bool GetTableData(std::vector<std::vector<std::string>>& data){
        data.clear();

        if (this->_res == nullptr)
        {
            return false;
        }

        auto fields_num = mysql_num_fields(this->_res);
        while (MYSQL_ROW row = mysql_fetch_row(this->_res))
        {
            std::vector<std::string> _v1;
            for (size_t index = 0; index < fields_num; index++)
            {
                _v1.push_back(row[index]);
            }
            data.push_back(_v1);
        }
        return true;
    }

private:
    MYSQL_RES* _res;
};

using sqlRes_ptr = std::shared_ptr<libMySQLResult>;

class libMySQL
{
public:
    libMySQL(/* args */);
    ~libMySQL();

    bool Init(const char* addr, const char* user_name, const char* passwd, const char* db_name, uint16_t port);
    void Release();

    sqlRes_ptr Query(const std::string& sql);
    inline const std::string& GetErrorString() { return this->_errorMessage; }

private:
    MYSQL _connect;
    std::string _errorMessage;
};


    
} // namespace wlb



