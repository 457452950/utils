#pragma once
#ifndef UTILS_WSINGLE_TCPSERVER_H
#define UTILS_WSINGLE_TCPSERVER_H

#include <vector>


#include "WChannel.h"
#include "WNetFactory.h"

namespace wlb::network {

class WSingleTcpServer {
public:
    explicit WSingleTcpServer();
    ~WSingleTcpServer();
    // no copyable
    WSingleTcpServer(const WSingleTcpServer &other)            = delete;
    WSingleTcpServer &operator=(const WSingleTcpServer &other) = delete;

    // class lifetimeUTILS_WSINGLE_TCPSERVER_H
    // bool Init();
    // void Close();
    // thread lifetime
    void Start();
    void Join();
    void Detach();

    // class methods
    bool AddAccepter(const std::string &IpAddress, uint16_t port, bool isv4);
    bool AddAccepter(const WEndPointInfo &local_info);

    void SetChannelFactory(WChannelFactory *factory);
    void SetSessionFactory(WSessionFactory *factory);

    using accept_cb_t = event_context_t::accept_cb_t;
    void SetOnAccept(accept_cb_t cb);
    void SetOnAccpetError(event_context_t::accept_error_cb_t cb);

    WTimer *NewTimer();

private:
    event_context_t                 contex_;
    std::vector<WAccepterChannel *> accepters_;
};

} // namespace wlb::network

#endif //UTILS_WSINGLE_TCPSERVER_H
