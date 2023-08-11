#pragma once
#ifndef WUTILS_NET_HTTP_METHOD_H
#define WUTILS_NET_HTTP_METHOD_H

namespace wutils::net::http {

enum Method {
    NONE,
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
    PATCH
};

}
#endif // WUTILS_NET_HTTP_METHOD_H
