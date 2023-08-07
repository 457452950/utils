#pragma once
#ifndef UTIL_TEST_TCP_ACONNECTION_H
#define UTIL_TEST_TCP_ACONNECTION_H


#include <iostream>
#include <csignal>
#include <utility>

#include "wutils/network/Network.h"

using namespace std;
using namespace wutils::network;


// #define CONTEXT event::EpollContext
#define CONTEXT event::SelectContext

#ifdef TEST_IPV4
#undef TEST_IPV4
#endif
#ifdef TEST_IPV6
#undef TEST_IPV6
#endif

#define TEST_IPV4

/**
 * test_aconnection
 */
namespace test_aconnection_config {

namespace srv {
namespace listen {
#ifdef TEST_IPV6
// constexpr char     *ip     = "0:0:0:0:0:0:0:0";
// constexpr int       port   = 4000;
// constexpr AF_FAMILY family = AF_FAMILY::INET6;
#elif defined TEST_IPV4
constexpr char     *ip     = "0.0.0.0";
constexpr int       port   = 12000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
#endif
} // namespace listen
} // namespace srv

namespace cli {

namespace connect {
#ifdef TEST_IPV6
// constexpr char     *ip     = "0:0:0:0:0:0:0:0";
// constexpr int       port   = 4000;
// constexpr AF_FAMILY family = AF_FAMILY::INET6;
#elif defined TEST_IPV4
constexpr char     *ip     = "0.0.0.0";
constexpr int       port   = 12000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
#endif
} // namespace connect

} // namespace cli

class TestSession : public AConnection::Listener {
public:
    explicit TestSession(std::shared_ptr<AConnection> ch_) : ch(std::move(ch_)) {
        ch->listener_      = this;
        buffer_.buffer     = new uint8_t[4096];
        buffer_.buffer_len = 4096;

        ch->AReceive(buffer_, AConnection::ARecvFlag::Keep);
    }
    ~TestSession() override { delete[] buffer_.buffer; }

    void OnDisconnect() override {
        LOG(LERROR, "session") << "disconnected";
        ch->listener_ = nullptr;
        this->ch.reset();
    }
    void OnReceived(uint64_t len) override {
        //        cout << "recv " << std::string((char *)buffer.buffer, (int)buffer.buffer_len) << " size " <<
        //        buffer.buffer_len
        //             << endl;
        ch->ASend({this->buffer_.buffer, len});
    }
    void OnSent(uint64_t len) override {}
    void OnError(wutils::SystemError error) override {
        LOG(LERROR, "session") << error;
        ch->listener_ = nullptr;
        this->ch.reset();
    }

    // private:
    Buffer                  buffer_;
    shared_ptr<AConnection> ch;
};

std::shared_ptr<TestSession> se;
std::shared_ptr<CONTEXT>     ep_;

inline auto ac_cb = [](const NetAddress &local, NetAddress *remote, unique_ptr<event::IOHandle> handler) {
    auto info = remote->Dump();
    auto lo   = local.Dump();
    LOG(LINFO, "accept") << "local " << std::get<0>(lo) << " " << std::get<1>(lo) << " accept : info "
                         << std::get<0>(info) << " " << std::get<1>(info);
    auto ch = std::make_shared<AConnection>(local, *remote, std::move(handler));
    se      = std::make_shared<TestSession>(ch);
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

    auto accp_channel = new AAcceptor(ep);
    auto remote       = new NetAddress;

    DEFER([=]() {
        delete accp_channel;
        delete remote;
    });

    if(!accp_channel->Open(local_ed.GetFamily())) {
        LOG(LERROR, "server") << wutils::SystemError::GetSysErrCode();
        abort();
    }

    if(!accp_channel->Start(local_ed, remote, AAcceptor::Keep)) {
        LOG(LERROR, "server") << wutils::SystemError::GetSysErrCode();
        abort();
    }

    accp_channel->OnAccept = std::bind(ac_cb, local_ed, remote, std::placeholders::_1);
    accp_channel->OnError  = err_cb;

    auto info = accp_channel->GetLocal().Dump();
    LOG(LINFO, "server") << std::get<0>(info) << " " << std::get<1>(info);

    ep->Loop();
    cout << wutils::SystemError::GetSysErrCode() << endl;

    if(se)
        se.reset();
    LOG(LERROR, "server") << "server thread end";
    // 激活客户端的 阻塞recv
}

void client_thread() {
    using namespace cli;
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);

    NetAddress cli_ed;
    assert(cli_ed.Assign(connect::ip, connect::port, connect::family));

    tcp::Socket cli;
    cli.Open(connect::family);
    bool res = cli.Connect(cli_ed);

    DEFER([&cli]() { cli.Close(); });

    if(!res) {
        LOG(LERROR, "client") << wutils::SystemError::GetSysErrCode();
        return;
    } else {
        LOG(LINFO, "client") << "connect ok";
        cli.Send((uint8_t *)"123123", 6);
    }

    std::thread thr1([&cli]() {
        // ::send(cli, "hello", 5, 0);
        int  total = 0;
        char buf[1500];
        while(true) {
            auto l = cli.Recv((uint8_t *)buf, 1500);
            if(l == 0) {
                LOG(LINFO, "client") << "dis connected.";
                break;
            }
            total += l;
            // clang-format off
//            cout
//                << "cli recv :" << std::string(buf, l)
//                << " total : " << total
//                << endl;
            // clang-format on
            if(total == 2190006) {
                LOG(LINFO, "client") << "stop.";
                ep_->Stop();
                return;
            }
        }
    });

    while(true) {
        static int i = 0;

        cli.Send((uint8_t *)"hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73);
        cli.Send((uint8_t *)"hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73);
        cli.Send((uint8_t *)"hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73);
        ++i;
        if(i == 10000) {
            break;
        }
        std::this_thread::sleep_for(2ms);
    }

    thr1.join();
    LOG(LINFO, "client") << "client thread end";
}


void handle_pipe(int signal) {
    cout << "signal" << endl;
    se.reset();
    ep_->Stop();
}


} // namespace test_aconnection_config


inline void test_aconnection() {
    using namespace test_aconnection_config;
    cout << "------------------------ test achannel ---------------------------" << endl;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    signal(SIGINT, handle_pipe);  // 自定义处理函数

    thread sr(server_thread);
    thread cl(client_thread);

    sr.join();
    cl.join();

    if(ep_) {
        ep_.reset();
    }

    cout << "------------------------ test achannel end ---------------------------" << endl;
}


#endif // UTIL_TEST_TCP_ACONNECTION_H
