#include "WNetWork/WSingleTcpServer.h"

#include <cassert>
#include <iostream>

namespace wlb::network {

static auto handle_read_callback = [](base_socket_type sock, WEpoll::user_data_ptr data) {
    auto *ch = (ReadChannel *)data;
    std::cout << "get channel call channel in [" << ch << "]" << std::endl;
    ch->ChannelIn();
    std::cout << "get channel call channel in end" << std::endl;
};

static auto handle_write_callback = [](base_socket_type sock, WEpoll::user_data_ptr data) {
    auto *ch_ = (WChannel *)data;
    auto  ch  = static_cast<WriteChannel *>(ch_);
    std::cout << "get channel call channel out [" << ch_ << "]" << std::endl;
    std::cout << "handle_write_callback get channel call channel out" << std::endl;
    // std::cout << " ch->ChannelOut & " << (int)&(ch->ChannelOut) << std::endl;
    ch->ChannelOut();
    std::cout << "handle_write_callback get channel call channel out end" << std::endl;
};

WSingleTcpServer::WSingleTcpServer() {
    this->contex_.event_handle_ = CreateNetHandle(HandleType::SELECT);
    // this->contex_.event_handle_         = CreateNetHandle(HandleType::EPOLL);
    this->contex_.event_handle_->read_  = handle_read_callback;
    this->contex_.event_handle_->write_ = handle_write_callback;
    this->contex_.max_read_size_        = 102400;
}
WSingleTcpServer::~WSingleTcpServer() { delete this->contex_.event_handle_; }

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

    SetSocketReuseAddr(l);
    SetSocketReusePort(l);
    SetSocketNoBlock(l);

    Bind(l, local_info);
    listen(l, 1024);

    assert(l != -1);

    auto *a = new WAccepterChannel(l, local_info, &this->contex_);
    accepters_.push_back(a);
}
void WSingleTcpServer::SetOnAccept(accept_cb_t cb) { this->contex_.onAccept = cb; }
void WSingleTcpServer::SetOnAccpetError(event_context_t::accept_error_cb_t cb) { this->contex_.onAcceptError = cb; }

void WSingleTcpServer::SetOnMessage(event_context_t::read_cb_t cb) { this->contex_.onRead = cb; }
void WSingleTcpServer::SetOnMessageError(event_context_t::read_error_cb_t cb) { this->contex_.onReadError = cb; }
void WSingleTcpServer::SetOnSendOrror(event_context_t::write_error_cb_t cb) { this->contex_.onWriteError = cb; }

} // namespace wlb::network
