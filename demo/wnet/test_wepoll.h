#ifndef UTILS_DEMO_WNET_TEST_WEPOLL_H
#define UTILS_DEMO_WNET_TEST_WEPOLL_H

#include <iostream>

#include "WNetWork/WNetWork.h"

using namespace std;
using namespace wlb::network;


/**
 * test_wepoll
 */

namespace test_wepoll_config {

namespace srv {
namespace listen {
constexpr char     *ip     = "::1";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET6;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace listen
} // namespace srv

namespace cli {

namespace connect {
constexpr char     *ip     = "::1";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET6;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace connect

} // namespace cli

struct test_s {
    std::function<void(int)> f;
    int                      n;
};

inline auto r_cb = [](socket_t sock, test_s *data) -> void {
    cout << "in" << sock << endl;

    WEndPointInfo en;
    int           res = Accept(sock, en);

    if(res == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "Accept ok" << endl;
    }

    auto [ip, port] = WEndPointInfo::Dump(en);
    cout << "client : ip " << ip << " port:" << port << endl;

    auto t = (test_s *)data;
    t->f(t->n);
};


inline void server_thread(std::shared_ptr<WEpoll<test_s>> ep) {
    using namespace srv;

    auto sock = MakeSocket(listen::family, listen::protol);
    SetSocketReuseAddr(sock);
    SetSocketReusePort(sock);

    WEndPointInfo lis;
    if(!lis.Assign(listen::ip, listen::port, listen::family)) {
        return;
    }
    int res = Bind(sock, lis);

    if(!res) {
        cout << "bind error : " << errno << endl;
        return;
    } else {
        cout << "bind ok" << endl;
    }

    res = ::listen(sock, 5);
    if(res == -1) {
        cout << "listen error : " << errno << endl;
        return;
    } else {
        cout << "listen ok " << res << endl;
    }

    test_s i{.f = [](int n) { cout << "heppy " << n << endl; }, .n = 3};
    auto   handler      = std::make_unique<WEventHandle<test_s>::WEventHandler>();
    handler->socket_    = sock;
    handler->user_data_ = &i;
    handler->handle_    = ep;
    handler->SetEvents(HandlerEventType::EV_IN);
    handler->Enable();

    ep->Loop();
}

inline void client_thread() {
    using namespace cli;

    WEndPointInfo cli_ed;
    if(!cli_ed.Assign(connect::ip, connect::port, connect::family)) {
        return;
    }
    auto cli = MakeSocket(connect::family, connect::protol);
    auto res = ConnectToHost(cli, cli_ed);

    if(!res) {
        cout << "[test_wepoll]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
    }
}

std::shared_ptr<WEpoll<test_s>> ep_;

void handle_pipe(int signal) {
    cout << "signal" << endl;
    ep_->Stop();
}

} // namespace test_wepoll_config


inline void test_wepoll() {
    using namespace test_wepoll_config;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    signal(SIGINT, handle_pipe); // 自定义处理函数

    cout << "test wepoll " << endl;
    auto ep = std::make_shared<WEpoll<test_s>>();
    ep_ = ep;
    ep->Init();
    ep->read_ = r_cb;

    thread sr(server_thread, ep);
    thread cl(client_thread);

    sr.join();
    cl.join();
}

#endif