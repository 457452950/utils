#pragma once
#ifndef WANFTS_ERROR_H
#define WANFTS_ERROR_H

#include <functional>
#include <ostream>
#include <string>
#include <unordered_map>

#include <cerrno>
#include <cstring>

namespace wutils {

class Error {
public:
    Error() {}
    Error(int no) : error_code_(no){};

    virtual int         Code() const { return error_code_; };
    virtual std::string Message() const = 0;

protected:
    int error_code_;
};

class SystemError : public Error {
public:
    SystemError(int code = 0) : Error(code) {}

    static SystemError GetSysErrCode() { return {errno}; }

    std::string Message() const override { return ::strerror(this->error_code_); }

    bool operator==(const SystemError &other) { return this->error_code_ == other.error_code_; }
    bool operator==(int code) { return this->error_code_ == code; }

    friend std::ostream &operator<<(std::ostream &o, const SystemError &error) {
        o << "{System error:code=" << error.error_code_ << ", message=" << error.Message() << "}";
        return o;
    }
};
} // namespace wutils

#endif // WANFTS_ERROR_H
