#pragma once
#ifndef WUTILS_NET_HTTP_SERVER_H
#define WUTILS_NET_HTTP_SERVER_H

#include <thread>
#include <cassert>

#include "Worker.h"

namespace wutils::net::http {

class Server : public Worker::Listener {
public:
    Server()  = default;
    ~Server() = default;

    void SetWorkingThread(int count);

    using MethodHandle = std::function<void(const shared_ptr<Request> request, shared_ptr<Response> response)>;

    void Get(std::string path, MethodHandle handle);
    void POST(std::string path, MethodHandle handle);
    void PUT(std::string path, MethodHandle handle);
    void DELETE(std::string path, MethodHandle handle);


    void Start(uint8_t port);
    void Start(std::string ip, uint8_t port);

private:
    void OnRequest(Method method, const shared_ptr<Request> request, shared_ptr<Response> response) override {
        auto path = request->GetUrl().GetPath();
        auto it   = methods_handle_map_.find(method);
        if(it == methods_handle_map_.end()) {
            // 405 Method_Not_Allowed
            return;
        };
        auto itt = it->second.find(std::string(path));
        if(itt == it->second.end()) {
            // 404 error
            return;
        }
        auto handle = itt->second;
        assert(handle);

        switch(method) {
        case GET:
            handle(request, response);
            break;
        case HEAD:
            break;
        case http::POST:
            handle(request, response);
            break;
        case http::PUT:
            handle(request, response);
            break;
        case http::DELETE:
            handle(request, response);
            break;
        case CONNECT:
            break;
        case OPTIONS:
            break;
        case TRACE:
            break;
        case PATCH:
            break;
        case UNKNOWN:
            break;
        }
    }

    std::unordered_map<Method, std::unordered_map<std::string, MethodHandle>> methods_handle_map_;
};


} // namespace wutils::net::http

#endif // WUTILS_NET_HTTP_SERVER_H
