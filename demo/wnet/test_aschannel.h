#pragma once
#ifndef UTIL_TEST_ASCHANNEL_H
#define UTIL_TEST_ASCHANNEL_H


#include <iostream>
#include <signal.h>
#include <utility>

#include "wutils/network/NetWork.h"

using namespace std;
using namespace wutils::network;


/**
 * test_aschannel
 */
namespace test_aschannel_config {

namespace srv {
namespace listen {
// constexpr char     *ip     = "0:0:0:0:0:0:0:0";
// constexpr int       port   = 4000;
// constexpr AF_FAMILY family = AF_FAMILY::INET6;
// constexpr AF_PROTOL protol = AF_PROTOL::TCP;
constexpr char     *ip     = "0.0.0.0";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace listen
} // namespace srv

namespace cli {

namespace connect {
// constexpr char     *ip     = "0:0:0:0:0:0:0:0";
// constexpr int       port   = 4000;
// constexpr AF_FAMILY family = AF_FAMILY::INET6;
// constexpr AF_PROTOL protol = AF_PROTOL::TCP;
constexpr char     *ip     = "0.0.0.0";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace connect

} // namespace cli
//
// class TestSession : public ASChannel::Listener {
// public:
//    TestSession(shared_ptr<ASChannel> ch_) : ch(std::move(ch_)) {
//        buffer.buffer  = new uint8_t[4096];
//        buffer.buf_len = 4096;
//
//        ch->ARecv(buffer);
//    }
//
//    void onChannelDisConnect() override {
//        std::cout << "disconnect" << std::endl;
//        this->ch.reset();
//    }
//    void onSend(ASChannel::ABuffer buf) override { ch->ARecv(this->buffer); }
//    void onReceive(ASChannel::ABuffer buf) override {
//        //        cout << "recv " << std::string((char *)buf.buffer, (int)buf.buf_len) << " size " << buf.buf_len <<
//        //        endl; ch->ARecv(this->buffer);
//        ch->ASend(buf);
//    }
//    void onError(wutils::SystemError error) override { std::cout << error << endl; }
//
//    // private:
//    std::shared_ptr<ASChannel> ch;
//    ASChannel::ABuffer         buffer;
//};
//
// std::shared_ptr<TestSession>    se;
// std::atomic_bool                active{true};
// std::shared_ptr<Epoll<IOEvent>> ep_;
//
// inline auto ac_cb = [](const EndPoint &local, const EndPoint &remote, io_hdle_p handler) {
//    auto info = EndPoint::Dump(remote);
//
//    cout << "recv : info " << std::get<0>(info) << " " << std::get<1>(info) << std::endl;
//    auto ch = std::make_shared<ASChannel>(local, remote, handler);
//
//    se = std::make_shared<TestSession>(ch);
//
//    ch->SetListener(se);
//};
//
//
// void server_thread() {
//    using namespace srv;
//
//    auto ep = std::make_shared<Epoll<IOEvent>>();
//    ep->Init();
//    ep_ = ep;
//
//    setCommonCallBack(ep.get());
//
//    EndPoint local_ed;
//    if(!local_ed.Assign(listen::ip, listen::port, listen::family)) {
//        return;
//    }
//
//    auto accp_channel = new Acceptor(ep);
//    accp_channel->Start(local_ed, true);
//    accp_channel->OnAccept = ac_cb;
//
//    ep->Loop();
//    cout << wutils::SystemError::GetSysErrCode() << endl;
//
//    // 激活客户端的 阻塞recv
//    se->ch->ASend({(uint8_t *)"1", 2});
//    delete accp_channel;
//}
//
// void client_thread() {
//    using namespace cli;
//
//    EndPoint cli_ed;
//    if(!cli_ed.Assign(connect::ip, connect::port, connect::family)) {
//        return;
//    }
//    auto cli = MakeSocket(connect::family, connect::protol);
//    bool res = ConnectToHost(cli, cli_ed);
//
//    if(!res) {
//        cout << "[test_tcpchannel]connect error : " << strerror(errno) << endl;
//        ::close(cli);
//        return;
//    } else {
//        cout << "connect ok" << endl;
//        ::send(cli, "123123", 6, 0);
//    }
//
//    std::thread thr1([cli]() {
//        //        ::send(cli, "hello", 5, 0);
//        int  total = 0;
//        char buf[1500];
//        while(active) {
//            auto l = ::recv(cli, buf, 1500, 0);
//            total += l;
//            // clang-format off
////            cout
////                 << "cli recv :" << std::string(buf, l)
////                << " total : " << total
////                << endl;
//            // clang-format on
//        }
//    });
//
//    int i = 100;
//    while(i--) {
//        auto len = ::send(cli, "hello123hello123hello123hello123hello123hello123hello123hello123hello123", 72, 0);
//        std::cout << "client send len " << len << std::endl;
//        std::this_thread::sleep_for(100ms);
//    }
//
//    //    Timer t(ep_);
//    //    t.OnTime = [cli, &t]() {
//    //        //        cout << std::chrono::duration_cast<std::chrono::milliseconds>(
//    //        //                        std::chrono::system_clock::now().time_since_epoch())
//    //        //                        .count()
//    //        //             << " ontime!!!" << endl;
//    //        static int i = 0;
//    //
//    //        ::send(cli, "hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73, 0);
//    //        ::send(cli, "hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73, 0);
//    //        ::send(cli, "hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73, 0);
//    //        ++i;
//    //        if(i == 2)
//    //            t.Stop();
//    //    };
//    //    t.Start(100, 1);
//
//    thr1.join();
//    ::close(cli);
//}
//
//
// void handle_pipe(int signal) {
//    cout << "signal" << endl;
//    ep_->Stop();
//    active.store(false);
//}
//

} // namespace test_aschannel_config


inline void test_aschannel() {
    using namespace test_aschannel_config;
    cout << "test aschannel " << endl;
    //
    //    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    //    signal(SIGINT, handle_pipe);  // 自定义处理函数
    //
    //    thread sr(server_thread);
    //    thread cl(client_thread);
    //
    //    sr.join();
    //    cl.join();
}


#endif // UTIL_TEST_ASCHANNEL_H
