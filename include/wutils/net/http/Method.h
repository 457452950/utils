#pragma once
#ifndef WUTILS_NET_HTTP_METHOD_H
#define WUTILS_NET_HTTP_METHOD_H

#include <vector>
#include <string>

#include "wutils/base/HeadOnly.h"

namespace wutils::net::http {

// doc: https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Methods
enum Method {
    // HTTP1.0
    GET,
    HEAD,
    POST,
    // HTTP1.1
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
    PATCH,
    UNKNOWN,
};

HEAD_ONLY const std::vector<std::string> MethodStr = {
        "GET",
        "HEAD",
        "POST",
        "PUT",
        "DELETE",
        "CONNECT",
        "OPTIONS",
        "TRACE",
        "PATCH",
};


} // namespace wutils::net::http
#endif // WUTILS_NET_HTTP_METHOD_H
