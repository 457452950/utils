#include "WNetWork/WSingleTcpServer.h"

#include <cassert>
#include <iostream>


namespace wlb::network {

static auto handle_read_callback = [](socket_t sock, WBaseChannel *ch) {
    // std::cout << "get channel call channel in [" << ch << "]" << std::endl;
    ch->ChannelIn();
    // std::cout << "get channel call channel in end" << std::endl;
};

static auto handle_write_callback = [](socket_t sock, WBaseChannel *ch) {
    // std::cout << "get channel call channel out [" << ch << "]" << std::endl;
    // std::cout << "handle_write_callback get channel call channel out" << std::endl;
    ch->ChannelOut();
    // std::cout << "handle_write_callback get channel call channel out end" << std::endl;
};

WSingleTcpServer::WSingleTcpServer() {
    this->contex_.event_handle_ = WNetFactory::CreateNetHandle(default_handle_type);

    this->contex_.event_handle_->read_  = handle_read_callback;
    this->contex_.event_handle_->write_ = handle_write_callback;

    this->contex_.onAccept = [](socket_t socket, WEndPointInfo &endpoint) { return true; };

    this->contex_.channel_factory_ = new WChannelFactory();
}

WSingleTcpServer::~WSingleTcpServer() {
}

void WSingleTcpServer::Start() { // this->contex_.event_handle_->Start(); 
}
void WSingleTcpServer::Join() { // this->contex_.event_handle_->Join(); 
}
void WSingleTcpServer::Detach() { // this->contex_.event_handle_->Detach(); 
}

bool WSingleTcpServer::AddAccepter(const std::string &IpAddress, uint16_t port, bool isv4) {
    auto info = WEndPointInfo::MakeWEndPointInfo(
            IpAddress, port, isv4 ? wlb::network::AF_FAMILY::INET : wlb::network::AF_FAMILY::INET6);
    if(info == nullptr) {
        return false;
    }

    this->AddAccepter(*info);
}

bool WSingleTcpServer::AddAccepter(const WEndPointInfo &local_info) {
    auto l = -1;
    if(local_info.GetFamily() == AF_FAMILY::INET) {
        l = MakeSocket(AF_FAMILY::INET, AF_PROTOL::TCP);
    } else {
        l = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    }

    if(l == -1) {
        std::cout << "WSingleTcpServer::AddAccepter(const WEndPointInfo &local_info) MakeSocket err " << std::endl;
        return false;
    }

    SetSocketReuseAddr(l);
    SetSocketReusePort(l);
    SetSocketNoBlock(l);

    auto ok = Bind(l, local_info);
    if(!ok) {
        std::cout << "WSingleTcpServer::AddAccepter(const WEndPointInfo &local_info) Bind err " << std::endl;
        return false;
    } else {
        std::cout << "WSingleTcpServer::AddAccepter(const WEndPointInfo &local_info) Bind ok " << std::endl;
    }

    int res = listen(l, 1024);
    if(res != 0) {
        std::cout << "WSingleTcpServer::AddAccepter(const WEndPointInfo &local_info) listen err " << std::endl;
        return false;
    } else {
        std::cout << "WSingleTcpServer::AddAccepter(const WEndPointInfo &local_info) listen ok " << std::endl;
    }

    // auto *a = new WAccepterChannel(l, local_info, &this->contex_);
    // accepters_.push_back(a);
    return true;
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
