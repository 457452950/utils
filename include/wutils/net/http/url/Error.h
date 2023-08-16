#pragma once
#ifndef WUTILS_NET_URL_ERROR_H
#define WUTILS_NET_URL_ERROR_H

#include <netdb.h>

#include "wutils/Error.h"

namespace wutils::net::http {

class NetURLErrorCategory : public error_category {
public:
    NetURLErrorCategory() = default;
    const char *name() const noexcept override { return "service"; }
    std::string message(int value) const override { return gai_strerror(value); }
};

Error make_error_code(int error) {
    static NetURLErrorCategory category;
    return {static_cast<int>(error), category};
}

} // namespace wutils::net::http


#endif // WUTILS_NET_DNS_ERROR_H
