#pragma once
#ifndef WUTILS_NET_DNS_ERROR_H
#define WUTILS_NET_DNS_ERROR_H

#include <unordered_map>

#include "wutils/Error.h"

namespace wutils::net::dns {

enum class eNetDNSError : int { OK = 0, SYSTEM_HOSTS_FILE_NOT_EXIST, HOSTS_FILE_PARSE_ERROR, UNKNOWN = INT32_MAX };

#define ERRMAPITEM(i, v)                                                                                               \
    { static_cast<int>(eNetDNSError::i), (char *)v }

const inline std::unordered_map<int, char *> ErrorMessageMap = {
        ERRMAPITEM(OK, "ok"),
        ERRMAPITEM(SYSTEM_HOSTS_FILE_NOT_EXIST, "system hosts file is not exist."),
        ERRMAPITEM(HOSTS_FILE_PARSE_ERROR, "hosts file parse error."),
        ERRMAPITEM(UNKNOWN, "unknown error")};


class NetWorkErrorCategory : public error_category {
public:
    NetWorkErrorCategory() = default;
    const char *name() const noexcept override { return "dns"; }
    std::string message(int value) const override {
        static const auto unkown = ErrorMessageMap.find(static_cast<int>(eNetDNSError::UNKNOWN));

        auto it = ErrorMessageMap.find(value);
        if(it == ErrorMessageMap.end()) {
            return unkown->second;
        }

        return it->second;
    }
};


Error make_error_code(eNetDNSError error) {
    static NetWorkErrorCategory category;
    return {static_cast<int>(error), category};
}


} // namespace wutils::net::dns

namespace std {
template <>
struct is_error_code_enum<wutils::net::dns::eNetDNSError> : public true_type {};
} // namespace std

#endif // WUTILS_NET_DNS_ERROR_H
