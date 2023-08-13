#pragma once
#ifndef WUTILS_NET_HTTP_WORKER_H
#define WUTILS_NET_HTTP_WORKER_H


#include <cstdint>
#include <string>
#include <functional>

#include "Request.h"

class Response;

namespace wutils::net::http {

class Worker {
public:
    class Listener {
    public:
        virtual void OnRequest(Method method, const Request &request, Response &response) = 0;

        virtual ~Listener() = default;
    } *listener_{nullptr};


    // block
    void work(std::string &ip, uint8_t port);

private:
};

} // namespace wutils::net::http

#endif // WUTILS_NET_HTTP_WORKER_H
