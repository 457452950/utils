#pragma once
#include <iostream>
#include <set>
#include <string>

#include "WNetWork/WChannel.h"
#include "WNetWork/WNetFactory.h"

using namespace wlb::network;

class MyChannel : public wlb::network::WChannel {
public:
    MyChannel(uint16_t buff_size) : WChannel(buff_size) {}


    void ChannelIn() override {
        WChannel::ChannelIn();
        std::cout << "MyChannel::ChannelIn" << std::endl;
    }
};

class MySession : public WBaseSession {
public:
    class Listener {
    public:
        virtual void onNewSession(MySession *session)   = 0;
        virtual void onSessionClose(MySession *session) = 0;
    };

public:
    MySession(Listener *listener) : listener_(listener) {}
    ~MySession() {}
    void init(std::unique_ptr<WChannel> channel) {
        this->channel_ = std::move(channel);
        this->channel_->SetListener(this);

        this->listener_->onSessionClose(this);
    }

private:
    void onChannelDisConnect() override {
        std::cout << "MySession::onChannelDisConnect" << std::endl;
        this->listener_->onSessionClose(this);
    }
    void onReceive(uint8_t *message, uint64_t message_len) override {
        channel_->Send(message, message_len);
        std::cout << "MySession::onReceive " << std::string((char *)message, message_len) << std::endl;
        channel_->CloseChannel();
    }
    void onError(uint64_t err_code) override { std::cout << "MySession::onError " << err_code << std::endl; }

    Listener *listener_{nullptr};
};

class SessionManager : public MySession::Listener {
public:
    static SessionManager *getInstance() {
        static SessionManager *instance = new SessionManager();
        return instance;
    }
    void onNewSession(MySession *session) override { set_.insert(session); }
    void onSessionClose(MySession *session) override { set_.erase(session); }

private:
    SessionManager() {}
    ~SessionManager() {}
    static inline std::set<MySession *> set_;
};


class MySessionFactory : public WSessionFactory {
public:
    void CreateSession(std::unique_ptr<WChannel> channel) override {
        auto newSession = new MySession(SessionManager::getInstance());
        newSession->init(std::move(channel));
    }
};
