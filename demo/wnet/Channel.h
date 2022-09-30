#pragma once
#include <iostream>

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
    void init(std::unique_ptr<WChannel> channel) {
        this->channel_ = std::move(channel);
        this->channel_->SetListener(this);
    }

private:
    void onChannelDisConnect() override { std::cout << "MySession::onChannelDisConnect" << std::endl; }
    void onReceive(uint8_t *message, uint64_t message_len) override {
        channel_->Send(message, message_len);
        std::cout << "MySession::onReceive" << std::endl;
        channel_->CloseChannel();
    }
    void onError(uint64_t err_code) override { std::cout << "MySession::onError " << err_code << std::endl; }
};

class MySessionFactory : public WSessionFactory {
public:
    std::shared_ptr<WBaseSession> CreateSession(std::unique_ptr<WChannel> channel) override {
        auto newSession = std::make_shared<MySession>();
        newSession->init(std::move(channel));
        return newSession;
    }
};
