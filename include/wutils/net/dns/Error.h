#pragma once
#ifndef WUTILS_NET_DNS_ERROR_H
#define WUTILS_NET_DNS_ERROR_H

#include <netdb.h>

#include "wutils/Error.h"
#include "wutils/network/base/Definition.h"

namespace wutils::net::dns {

class NetDNSErrorCategory : public error_category {
public:
    NetDNSErrorCategory() = default;
    const char *name() const noexcept override { return "dns"; }
    std::string message(int value) const override { return gai_strerror(value); }
};

Error make_error_code(int error) {
    static NetDNSErrorCategory category;
    return {static_cast<int>(error), category};
}

} // namespace wutils::net::dns


#endif // WUTILS_NET_DNS_ERROR_H
