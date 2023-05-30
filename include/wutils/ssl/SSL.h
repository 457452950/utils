#pragma once
#ifndef UTIL_SSL_H
#define UTIL_SSL_H

// TODO:Use openssl tsl
// 私钥生成：openssl genrsa -out server.key 2048
// 证书生成，（公钥）：openssl req -new -key server.key -out server.csr
// 自签名：openssl x509 -req -days 365 -in server.csr signkey server.key -out server.crt

#endif // UTIL_SSL_H
