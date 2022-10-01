#include "WNetWork/WSingleTcpServer.h"

#include <cassert>
#include <iostream>


namespace wlb::network {

static auto handle_read_callback = [](base_socket_type sock, WBaseChannel *ch) {
    std::cout << "get channel call channel in [" << ch << "]" << std::endl;
    ch->ChannelIn();
    std::cout << "get channel call channel in end" << std::endl;
};

static auto handle_write_callback = [](base_socket_type sock, WBaseChannel *ch) {
    std::cout << "get channel call channel out [" << ch << "]" << std::endl;
    std::cout << "handle_write_callback get channel call channel out" << std::endl;
    ch->ChannelOut();
    std::cout << "handle_write_callback get channel call channel out end" << std::endl;
};

WSingleTcpServer::WSingleTcpServer() {
    this->contex_.event_handle_ = WNetFactory::CreateNetHandle(default_handle_type);

    this->contex_.event_handle_->read_  = handle_read_callback;
    this->contex_.event_handle_->write_ = handle_write_callback;

    this->contex_.onAccept = [](base_socket_type socket, WEndPointInfo &endpoint) { return true; };

    this->contex_.channel_factory_ = new WChannelFactory();
}

WSingleTcpServer::~WSingleTcpServer() {
    if(this->contex_.event_handle_ != nullptr) {
        delete this->contex_.event_handle_;
        this->contex_.event_handle_ = nullptr;
    }
}

void WSingleTcpServer::Start() { this->contex_.event_handle_->Start(); }
void WSingleTcpServer::Join() { this->contex_.event_handle_->Join(); }
void WSingleTcpServer::Detach() { this->contex_.event_handle_->Detach(); }

bool WSingleTcpServer::AddAccepter(const std::string &IpAddress, uint16_t port, bool isv4) {
    this->AddAccepter({IpAddress, port, isv4});
}

bool WSingleTcpServer::AddAccepter(const WEndPointInfo &local_info) {
    auto l = -1;
    if(local_info.isv4) {
        l = MakeSocket(AF_FAMILY::INET, AF_PROTOL::TCP);
    } else {
        l = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    }

    if(l == -1) {
        return false;
    }

    SetSocketReuseAddr(l);
    SetSocketReusePort(l);
    SetSocketNoBlock(l);

    Bind(l, local_info);
    listen(l, 1024);

    assert(l != -1);

    auto *a = new WAccepterChannel(l, local_info, &this->contex_);
    accepters_.push_back(a);
}

void WSingleTcpServer::SetChannelFactory(WChannelFactory *factory) {
    if(factory) {
        this->contex_.channel_factory_ = factory;
    }
}

void WSingleTcpServer::SetSessionFactory(WSessionFactory *factory) {
    if(factory) {
        this->contex_.session_factory_ = factory;
    }
}

void WSingleTcpServer::SetOnAccept(accept_cb_t cb) { this->contex_.onAccept = cb; }
void WSingleTcpServer::SetOnAccpetError(event_context_t::accept_error_cb_t cb) { this->contex_.onAcceptError = cb; }

WTimer *WSingleTcpServer::NewTimer() { return new WTimer(this->contex_.event_handle_); }

} // namespace wlb::network
