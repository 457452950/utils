//
// Created by wang on 22-5-5.
//

#ifndef UTILS_DEMO__TESTSESSION_H_
#define UTILS_DEMO__TESTSESSION_H_

#include <iostream>
#include "WNetWork/WNetWork.h"

using namespace std;
using namespace wlb::NetWork;

class Session : public WBaseSession {
public:
    explicit Session(WBaseSession::Listener *listener) : WBaseSession(listener) {}
    ~Session() override = default;

    void OnConnectionMessage(const std::string &receive_message) override {
//        cout << "recv msg : " << receive_message << endl;
        this->connection_->Send(receive_message);
    };
    void OnConnectionClosed() override {
//        cout << "close" << endl;
        this->connection_->Clear();
    };
    void OnConnectionShutdown() override {
        cout << "shutdown" << this->connection_->isConnected() << endl;
        this->connection_->Send("123");
//        this->connection_->Clear();
    };
    void OnConnectionError() override {
        cout << "error" << endl;
        this->connection_->Clear();
    };

    bool AcceptConnection(const WEndPointInfo &peer_info) override {
        cout << "ip:" << peer_info.ip_address << " port:" << peer_info.port << endl;
        return true;
    };
    WBaseConnection *CreateConnection() override {
        auto c = new WFixedBufferConnection(this);
        c->Init(this->handler_, 2048, 3);
        return c;
    };
};

#endif //UTILS_DEMO__TESTSESSION_H_
