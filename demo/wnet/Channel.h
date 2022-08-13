#pragma once
#include <iostream>
#include "WNetWork/WChannel.h"

using namespace wlb::network;

class MyChannel : public wlb::network::WChannel {
public:
    MyChannel(base_socket_type socket, WEndPointInfo &remote_endpoint) : WChannel(socket, remote_endpoint) {}
    void ChannelIn() override {
        WChannel::ChannelIn();
        std::cout << "MyChannel::ChannelIn" << std::endl;
    }
};



