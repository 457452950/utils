#include "WNetWork/WMultiTcpServer.hpp"
#include <iostream>

namespace wlb::network {


/////////////////////////////////
// timer

WTimerHandler *timeHandler = new WTimerEpoll;


////////////////////////////////////////////////////////////////
// server

bool WMultiTcpServer::Init(uint16_t threads) {
    this->_threadsCount = threads < 1 ? 1 : threads;
    this->_servers.clear();

    for(size_t i = 0; i < this->_threadsCount; ++i) {
        WSingleTcpServer *the = new(std::nothrow) WSingleTcpServer();
        if(the == nullptr || !the->Init()) {
            return false;
        }

        this->_servers.push_back(the);
    }

    if(!timeHandler->Init()) {
        return false;
    }

    this->_timer = new(std::nothrow) WServerTimer(this);
    if(this->_timer == nullptr) {
        return false;
    }

    // this->timer_->Start(1000, 1000);


    return true;
}

void WMultiTcpServer::Close() {
    this->_isRunning = false;

    // 采用迭代器，防止init失败时，this->_threadsCount失效
    for(auto it : this->_servers) {
        it->Close();
    }
}

void WMultiTcpServer::Destroy() {
    // 采用迭代器，防止init失败时，this->_threadsCount失效
    for(auto it : this->_servers) {
        it->Destroy();
        delete it;
    }
    this->_servers.clear();
    if(this->_timerThread != nullptr) {
        delete this->_timerThread;
        this->_timerThread = nullptr;
    }

    delete this->_timer;
}

void WMultiTcpServer::run() {
    this->_isRunning = true;

    for(auto it : this->_servers) {
        it->run();
    }

    this->_timerThread = new std::thread(&WMultiTcpServer::Loop, this);
}

void WMultiTcpServer::WaitForQuit() {
    for(auto it : this->_servers) {
        it->WaitForQuit();
    }
}

bool WMultiTcpServer::AddAccepter(const std::string &IpAddress, uint16_t port) {
    for(auto it : this->_servers) {
        if(!it->AddAccepter(IpAddress, port)) {
            return false;
        }
    }

    return true;
}

void WMultiTcpServer::Loop() {
    while(this->_isRunning) {
        timeHandler->GetAndEmitTimer(-1);
    }
}

void WMultiTcpServer::OnTime(WTimer *timer) {
    if(this->_timer == timer) {
        int index = 0;
        for(auto it : this->_servers) {
            std::cout << index++ << " " << it->GetActiveConnectionCount() << " " << it->GetTempConnectionCount()
                      << std::endl;
        }
    }
}
bool WMultiTcpServer::AddAccepter(const WEndPointInfo &peer_info) {
    for(auto it : this->_servers) {
        if(!it->AddAccepter(peer_info)) {
            return false;
        }
    }

    return true;
}

} // namespace wlb::network
