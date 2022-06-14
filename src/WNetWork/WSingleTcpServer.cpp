#include "WNetWork/WSingleTcpServer.hpp"

#include <cassert>
#include <iostream>

namespace wlb::network {

bool WSingleTcpServer::Init() {
    this->session_temp_.Init();
    this->session_list_.Init();

    this->accepter_map_.clear();

    this->handler_ = CreateNetworkHandlerAndInit(128);
    if(this->handler_ == nullptr) {
        return false;
    }

    if(!this->IncreaseConnectionTemp()) {
        return false;
    }

    return true;
}

void WSingleTcpServer::Close() {
    this->running_ = false;

    this->session_temp_.clear();
    this->session_list_.clear();

    this->handler_->Close();
}

void WSingleTcpServer::Destroy() {
    if(this->work_thread_ != nullptr) {
        delete this->work_thread_;
        this->work_thread_ = nullptr;
    }

    this->session_temp_.clear();
    this->session_list_.clear();

    for(auto it : this->accepter_map_) {
        delete it.second;
    }
    this->accepter_map_.clear();

    if(this->handler_ != nullptr) {
        delete this->handler_;
        this->handler_ = nullptr;
    }
}

void WSingleTcpServer::run() {
    this->running_     = true;
    this->work_thread_ = new(std::nothrow) std::thread(&WSingleTcpServer::Loop, this);
}

void WSingleTcpServer::WaitForQuit() {
    if(this->work_thread_ != nullptr && this->work_thread_->joinable()) {
        this->work_thread_->join();
    }
}

bool WSingleTcpServer::AddAccepter(const std::string &IpAddress, uint16_t port) {
    WNetAccepter *acc = new(std::nothrow) WNetAccepter(this);
    if(acc == nullptr) {
        return false;
    }

    if(!acc->Init(this->handler_, IpAddress, port)) {
        delete acc;
        return false;
    }

    accepter_map_.insert(std::make_pair(acc->GetListenSocket(), acc));
    return true;
}

bool WSingleTcpServer::OnNewConnection(base_socket_type socket, const WEndPointInfo &peerInfo) {
    if(this->session_temp_.empty() && !this->IncreaseConnectionTemp()) {
        // can't new connection
        std::cout << "cant new connection" << std::endl;
        assert(0);
    }

    auto          node    = this->session_temp_.front();
    WBaseSession *session = node->val;
    if(session->IsConnected()) {
        std::cout << "connection is connected" << std::endl;
        assert(!session->IsConnected());
    }

    if(!session->SetConnectedSocket(socket, peerInfo)) {
        session->Clear();
        return false;
    }

    return true;
}

void WSingleTcpServer::OnNewSession(SessionNode *node) {
    this->session_temp_.erase(node);
    this->session_list_.push_back(node);
}
void WSingleTcpServer::OnSessionClosed(SessionNode *node) { this->session_temp_.strong_push_back(node); }

void WSingleTcpServer::Loop() {
    while(running_) {
        this->handler_->GetAndEmitEvents(-1);
    }
}

bool WSingleTcpServer::IncreaseConnectionTemp() {
    for(size_t index = 0; index < this->connections_increase_; ++index) {
        auto *session = CreateNewSessionNodeAndInit(this, this->handler_);

        if(session == nullptr) {
            return false;
        }

        this->session_temp_.push_front(session);
    }
    return true;
}

WSingleTcpServer::~WSingleTcpServer() { this->Destroy(); }

bool WSingleTcpServer::AddAccepter(const WEndPointInfo &local_info) {
    WNetAccepter *acc = new(std::nothrow) WNetAccepter(this);
    if(acc == nullptr) {
        return false;
    }

    if(!acc->Init(this->handler_, local_info)) {
        delete acc;
        return false;
    }

    accepter_map_.insert(std::make_pair(acc->GetListenSocket(), acc));
    return true;
}

} // namespace wlb::network
