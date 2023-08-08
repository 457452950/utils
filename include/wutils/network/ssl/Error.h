#pragma once
#ifndef UTIL_NETWORK_SSL_ERROR_H
#define UTIL_NETWORK_SSL_ERROR_H

#include <openssl/err.h>

#include "wutils/base/HeadOnly.h"
#include "wutils/network/Error.h"

namespace wutils::network::ssl {

#define SSL_ERR_MSG_LEN (1024)

class SslErrorCategory : public error_category {
public:
    SslErrorCategory() = default;
    const char *name() const noexcept override { return "ssl"; }
    std::string message(int value) const override {
        char msg[SSL_ERR_MSG_LEN]{0};

        ERR_error_string_n(static_cast<unsigned long>(value), msg, SSL_ERR_MSG_LEN);

        return msg;
    }
};


HEAD_ONLY Error make_ssl_error_code(unsigned long error) {
    static SslErrorCategory category;
    return {static_cast<int>(error), category};
}


} // namespace wutils::network::ssl

#endif // UTIL_NETWORK_SSL_ERROR_H
