#pragma once
#ifndef UTIL_NETWORK_SSL_ERROR_H
#define UTIL_NETWORK_SSL_ERROR_H

#include <openssl/err.h>

#include "wutils/base/HeadOnly.h"
#include "wutils/network/Error.h"

namespace wutils::network::ssl {


class SslErrorCategory : public error_category {
public:
    SslErrorCategory() = default;
    const char *name() const noexcept override { return "ssl"; }
    std::string message(int value) const override {
        char msg[1024];

        ERR_error_string_n(static_cast<unsigned long>(value), msg, 1024);

        return {msg, 1024};
    }
};


HEAD_ONLY Error make_ssl_error_code(unsigned long error) {
    static SslErrorCategory category;
    return {static_cast<int>(error), category};
}


} // namespace wutils::network::ssl

#endif // UTIL_NETWORK_SSL_ERROR_H
