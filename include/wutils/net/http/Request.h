#pragma once
#ifndef WUTILS_NET_HTTP_REQUEST_H
#define WUTILS_NET_HTTP_REQUEST_H

#include <string>
#include <map>


#include "wutils/SharedPtr.h"
#include "Method.h"
#include "Header.h"
#include "Url.h"

namespace wutils::net::http {

using Headers = std::map<std::string, std::string>;

class Request {
public:
    Request()  = default;
    ~Request() = default;

    const Url &GetUrl() const { return url_; }

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
    Method              method_;
    Url                 url_;
    Headers             headers_;
    unique_ptr<uint8_t> data_{nullptr};
    uint32_t            data_len_{0};
};

} // namespace wutils::net::http

#endif // WUTILS_NET_HTTP_REQUEST_H
