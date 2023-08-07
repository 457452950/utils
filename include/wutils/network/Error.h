#pragma once
#ifndef WUTILS_NETWORK_ERROR_H
#define WUTILS_NETWORK_ERROR_H

#include "wutils/Error.h"

#include <unordered_map>

namespace wutils::network {

enum class eNetWorkError : int { OK = 0, CONTEXT_TOO_MUCH_HANDLE, CONTEXT_CANT_FOUND_HANDLE, UNKNOWN = INT32_MAX };

#define ERRMAPITEM(i, v)                                                                                               \
    { static_cast<int>(eNetWorkError::i), (char *)v }

const inline std::unordered_map<int, char *> ErrorMessageMap = {
        ERRMAPITEM(OK, "ok"),
        ERRMAPITEM(CONTEXT_TOO_MUCH_HANDLE, "context has too much handle"),
        ERRMAPITEM(CONTEXT_CANT_FOUND_HANDLE, "context cant found handle"),
        ERRMAPITEM(UNKNOWN, "unknown error")};


class NetWorkErrorCategory : public error_category {
public:
    NetWorkErrorCategory() = default;
    const char *name() const noexcept override { return "network"; }
    std::string message(int value) const override {
        static const auto unkown = ErrorMessageMap.find(static_cast<int>(eNetWorkError::UNKNOWN));

        auto it = ErrorMessageMap.find(value);
        if(it == ErrorMessageMap.end()) {
            return unkown->second;
        }

        return it->second;
    }
};


Error make_error_code(eNetWorkError error) {
    static NetWorkErrorCategory category;
    return {static_cast<int>(error), category};
}

} // namespace wutils::network

namespace std {
template <>
struct is_error_code_enum<wutils::network::eNetWorkError> : public true_type {};
} // namespace std

#endif // WUTILS_NETWORK_ERROR_H
