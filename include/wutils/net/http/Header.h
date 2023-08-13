#pragma once
#ifndef WUTILS_NET_HTTP_HEADER_H
#define WUTILS_NET_HTTP_HEADER_H

#include <string>
#include <unordered_set>

#include "wutils/base/HeadOnly.h"

namespace wutils::net::http {

// doc: https://developer.mozilla.org/zh-CN/docs/web/http/headers

// Connection: keep-alive
HEAD_ONLY const std::string CONNECTION            = "connection";
HEAD_ONLY const std::string CONNECTION_KEEP_ALIVE = "keep-alive";
HEAD_ONLY const std::string CONNECTION_CLOSE      = "close";

/**
 * @parameters
 * 一系列用逗号隔开的参数，每一个参数由一个标识符和一个值构成，并使用等号 ('=') 隔开。下述标识符是可用的：
 * @timeout：指定了一个空闲连接需要保持打开状态的最小时长（以秒为单位）。需要注意的是，
 * 如果没有在传输层设置 keep-alive TCP message 的话，大于 TCP 层面的超时设置会被忽略。
 * @max：在连接关闭之前，在此连接可以发送的请求的最大值。在非管道连接中，除了 0 以外，
 * 这个值是被忽略的，因为需要在紧跟着的响应中发送新一次的请求。HTTP 管道连接则可以用它来限制管道的使用。
 */
// Keep-Alive: timeout=5, max=1000
HEAD_ONLY const std::string KEEP_ALIVE          = "keep-alive";
HEAD_ONLY const std::string KEEP_ALIVE_TIME_OUT = "timeout";
HEAD_ONLY const std::string KEEP_ALIVE_MAX      = "max";

// Date: <day-name>, <day> <month> <year> <hour>:<minute>:<second> GMT
// Date: Wed, 21 Oct 2015 07:28:00 GMT
HEAD_ONLY const std::string DATA = "date";

// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q = 0.8
HEAD_ONLY const std::string ACCEPT = "accept";

// Host 首部字段在 HTTP/1.1 规范内是唯一一个必须被包含在请求内的首部字段。
// doc: https://cloud.tencent.com/developer/article/1586859
// Host: developer.mozilla.org
HEAD_ONLY const std::string HOST = "host";

// 实体头

// Allow: GET, POST, HEAD
HEAD_ONLY const std::string ALLOW            = "allow";
// Accept-Encoding: gzip, deflate
// Content-Encoding: gzip
HEAD_ONLY const std::string CONTENT_ENCODING = "content-encoding";
// Content-Language: de-DE, en-CA
HEAD_ONLY const std::string CONTENT_LANGUAGE = "content-language";
// Content-Length 的实体标头指服务器发送给客户端的实际主体大小，以字节为单位。
// Content-Length: 3000
HEAD_ONLY const std::string CONTENT_LENGTH   = "content-length";
// Content-MD5: e10adc3949ba59abbe56e057f20f883e
HEAD_ONLY const std::string CONTENT_MD5      = "content-md5";
// Content-Range: bytes 200-1000/67589
HEAD_ONLY const std::string CONTENT_RANGE    = "content-range";


} // namespace wutils::net::http

#endif // WUTILS_NET_HTTP_HEADER_H
