#pragma once
#ifndef WUTILS_NET_HTTP_REQUEST_H
#define WUTILS_NET_HTTP_REQUEST_H

#include <string>
#include <map>

#include "Method.h"
#include "Header.h"
#include "Url.h"

namespace wutils::net::http {

using Headers = std::map<std::string, std::string>;

class Request {
public:
    Request()  = default;
    ~Request() = default;

    Url Url() const {}

    std::string_view GetHeader(const std::string &key) {
        auto it = headers_.find(key);
        if(it == headers_.end()) {
            return {};
        } else {
            return it->second;
        }
    }
    const Headers &GetHeaders() { return headers_; }

private:
    Method  method_;
    Headers headers_;
};

} // namespace wutils::net::http

#endif // WUTILS_NET_HTTP_REQUEST_H
