#pragma once
#ifndef UTIL_TEST_ECHO_TCP_H
#define UTIL_TEST_ECHO_TCP_H


#include <iostream>
#include <utility>

#include "wutils/network/Network.h"

using namespace std;
using namespace wutils::network;

#define CONTEXT event::EpollContext
// #define CONTEXT event::SelectContext

/**
 * test_tcp_echo
 */
namespace test_tcp_echo_config {

namespace srv {
namespace listen {
// constexpr char     *ip     = "0:0:0:0:0:0:0:0";
// constexpr int       port   = 4000;
// constexpr AF_FAMILY family = AF_FAMILY::INET6;
constexpr char     *ip      = "0.0.0.0";
constexpr int       port    = 4000;
constexpr AF_FAMILY family  = AF_FAMILY::INET;
constexpr char     *ip2     = "0.0.0.0";
constexpr int       port2   = 4001;
constexpr AF_FAMILY family2 = AF_FAMILY::INET;
} // namespace listen
} // namespace srv


class TestSession : public Connection::Listener {
public:
    explicit TestSession(std::shared_ptr<Connection> ch_) : ch(std::move(ch_)) { ch->listener_ = this; }

    void OnDisconnect() override {
        std::cout << "disconnect" << std::endl;
        this->ch.reset();
    }
    void OnReceive(Data data) override {
        //        cout << "recv " << std::string((char *)message, (int)message_len) << " size " << message_len << endl;
        ch->Send(data.data, data.bytes);
    }
    void OnError(wutils::SystemError error) override {
        std::cout << error << endl;
        this->ch.reset();
    }

    // private:
    std::shared_ptr<Connection> ch;
};

class TestASession : public AConnection::Listener {
public:
    explicit TestASession(std::shared_ptr<AConnection> ch_) : ch(std::move(ch_)) {
        ch->listener_      = this;
        buffer_.buffer     = new uint8_t[4096];
        buffer_.buffer_len = 4096;

        ch->AReceive(buffer_, AConnection::ARecvFlag::Keep);
    }
    ~TestASession() override { delete[] buffer_.buffer; }

    void OnDisconnect() override {
        std::cout << "disconnect" << std::endl;
        this->ch.reset();
    }
    void OnReceived(uint64_t len) override {
        //        cout << "recv " << std::string((char *)buffer.buffer, (int)buffer.buffer_len) << " size " <<
        //        buffer.buffer_len
        //             << endl;
        ch->ASend({buffer_.buffer, len});
    }
    void OnSent(uint64_t len) override {}
    void OnError(wutils::SystemError error) override {
        std::cout << error << endl;
        this->ch.reset();
    }

    // private:
    Buffer                  buffer_;
    shared_ptr<AConnection> ch;
};


std::shared_ptr<CONTEXT>      ep_;
std::shared_ptr<TestSession>  se;
std::shared_ptr<TestASession> se2;


inline auto ac_cb = [](const NetAddress &local, const NetAddress &remote, unique_ptr<event::IOHandle> handler) {
    auto info = remote.Dump();

    cout << "accept : info " << std::get<0>(info) << " " << std::get<1>(info) << std::endl;
    auto ch = std::make_shared<Connection>(local, remote, std::move(handler));
    se      = std::make_shared<TestSession>(ch);
};
inline auto ac2_cb = [](const NetAddress &local, const NetAddress &remote, unique_ptr<event::IOHandle> handler) {
    auto info = remote.Dump();

    cout << "accept : info " << std::get<0>(info) << " " << std::get<1>(info) << std::endl;
    auto ch = std::make_shared<AConnection>(local, remote, std::move(handler));
    se2     = std::make_shared<TestASession>(ch);
};
inline auto err_cb = [](wutils::SystemError error) { std::cout << error << std::endl; };


void server_thread() {
    using namespace srv;

    auto ep = CONTEXT::Create();
    ep->Init();
    ep_ = ep;

    NetAddress local_ed;
    if(!local_ed.Assign(listen::ip, listen::port, listen::family)) {
        return;
    }
    NetAddress local2_ed;
    if(!local2_ed.Assign(listen::ip2, listen::port2, listen::family2)) {
        return;
    }

    auto accp_channel  = new Acceptor(ep);
    auto accp_channel2 = new Acceptor(ep);
    DEFER([=]() {
        delete accp_channel;
        delete accp_channel2;
    });

    if(!accp_channel->Open(local_ed.GetFamily())) {
        LOG(LERROR, "server") << wutils::SystemError::GetSysErrCode();
        abort();
    }


    if(!accp_channel->Start(local_ed)) {
        LOG(LERROR, "server") << wutils::SystemError::GetSysErrCode();
        abort();
    }
    accp_channel->OnAccept = ac_cb;
    accp_channel->OnError  = err_cb;

    if(!accp_channel2->Open(local2_ed.GetFamily())) {
        LOG(LERROR, "server") << wutils::SystemError::GetSysErrCode();
        abort();
    }

    if(!accp_channel2->Start(local2_ed)) {
        LOG(LERROR, "server") << wutils::SystemError::GetSysErrCode();
        abort();
    }
    accp_channel2->OnAccept = ac2_cb;
    accp_channel2->OnError  = err_cb;

    ep->Loop();
    cout << wutils::SystemError::GetSysErrCode() << endl;

    LOG(LERROR, "server") << "server thread end";
}

void handle_pipe(int signal) {
    LOG(LINFO, "signal") << "signal " << signal;
    ep_->Stop();
    se.reset();
}


} // namespace test_tcp_echo_config


inline void test_tcp_echo() {
    using namespace test_tcp_echo_config;
    cout << "-------------------- test tcp echo --------------------" << endl;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    signal(SIGINT, handle_pipe);  // 自定义处理函数

    thread sr(server_thread);

    sr.join();

    if(ep_)
        ep_.reset();
    if(se)
        se.reset();
    if(se2)
        se2.reset();
}


#endif // UTIL_TEST_ECHO_TCP_H
