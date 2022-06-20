#pragma once

#include <vector>

#include "WChannel.h"

namespace wlb::network {

class WSingleTcpServer {
public:
    explicit WSingleTcpServer();
    ~WSingleTcpServer();
    // no copyable
    WSingleTcpServer(const WSingleTcpServer &other)            = delete;
    WSingleTcpServer &operator=(const WSingleTcpServer &other) = delete;

    // class lifetime
    // bool Init();
    // void Close();
    // thread lifetime
    void Start();
    void Join();
    void Detach();

    // class methods
    bool AddAccepter(const std::string &IpAddress, uint16_t port, bool isv4);
    bool AddAccepter(const WEndPointInfo &local_info);

    typedef bool (*accept_cb_t)(base_socket_type socket, WEndPointInfo &endpoint);
    void SetOnAccept(accept_cb_t cb);
    void SetOnAccpetError(event_context_t::accept_error_cb_t cb);

    void SetOnMessage(event_context_t::read_cb_t cb);
    void SetOnMessageError(event_context_t::read_error_cb_t cb);
    void SetOnSendOrror(event_context_t::write_error_cb_t cb);

    WTimer* NewTimer();

private:
    event_context_t                 contex_;
    std::vector<WAccepterChannel *> accepters_;
};

} // namespace wlb::network
