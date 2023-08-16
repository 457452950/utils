#pragma once
#ifndef WUTILS_NET_HTTP_URL_SCHEME_H
#define WUTILS_NET_HTTP_URL_SCHEME_H

#include <future>
#include <list>
#include <netdb.h> // gnu

#include "Error.h"
#include "wutils/base/HeadOnly.h"
#include "wutils/network/base/Native.h"     // HtoN
#include "wutils/network/base/Definition.h" // AF_FAMILY

namespace wutils::net::http {

HEAD_ONLY const std::string_view URL_SCHEME_HTTP_HEAD  = "http";
HEAD_ONLY const std::string_view URL_SCHEME_HTTPS_HEAD = "https";
HEAD_ONLY const std::string_view URL_SCHEME_WS_HEAD    = "ws";
HEAD_ONLY const std::string_view URL_SCHEME_WSS_HEAD   = "wss";
HEAD_ONLY const std::string_view URL_SCHEME_FTP_HEAD   = "ftp";

HEAD_ONLY const std::unordered_set<std::string_view> SchemeMap = {
        URL_SCHEME_HTTP_HEAD,
        URL_SCHEME_HTTPS_HEAD,
        URL_SCHEME_WS_HEAD,
        URL_SCHEME_WSS_HEAD,
        URL_SCHEME_FTP_HEAD,
};

HEAD_ONLY std::unordered_map<std::string, uint8_t> SchemePortMap = {
        {std::string{URL_SCHEME_FTP_HEAD}, 21},
        {std::string{URL_SCHEME_HTTP_HEAD}, 80},
        {std::string{URL_SCHEME_WS_HEAD}, 80},
        {std::string{URL_SCHEME_HTTPS_HEAD}, 443},
        {std::string{URL_SCHEME_WSS_HEAD}, 443},
};


class ServiceHelper {
public:
    class Result {
    public:
        explicit Result(const std::string &service, int error) : service_(service) {
            error_ = http::make_error_code(error);
        }
        explicit Result(const std::string &service, addrinfo *info) : service_(service) {
            if(info == nullptr) {
                return;
            }

            if(info->ai_addr->sa_family == network::AF_FAMILY::INET) {
                network::NtoHS(((sockaddr_in *)(info->ai_addr))->sin_port, &port_);
            } else {
                network::NtoHS(((sockaddr_in6 *)(info->ai_addr))->sin6_port, &port_);
            }
        }

        bool         Success() const { return port_ != 0; }
        std::string  Service() const { return service_; }
        uint16_t     Get() const { return port_; }
        const Error &GetError() const { return error_; }

    private:
        std::string service_;
        uint16_t    port_{0};
        Error       error_;
    };

    // check in /etc/service
    static std::future<Result> GetServiceDefaultPort(const std::string &service) {
        return std::async([=]() -> Result {
            addrinfo  input_info{0};
            addrinfo *result;

            input_info.ai_flags = AI_PASSIVE;

            int err = ::getaddrinfo(nullptr, service.c_str(), &input_info, &result);
            if(err != 0) {
                // false
                return Result{service, err};
            }

            Result res{service, result};
            freeaddrinfo(result);

            return res;
        });
    }
}; // namespace wutils::net::dns


} // namespace wutils::net::http

#endif // WUTILS_NET_HTTP_URL_SCHEME_H
