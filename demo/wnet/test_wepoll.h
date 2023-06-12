#ifndef UTILS_DEMO_WNET_TEST_WEPOLL_H
#define UTILS_DEMO_WNET_TEST_WEPOLL_H

#include <iostream>

#include "wutils/network/NetWork.h"

using namespace std;
using namespace wutils::network;


/**
 * test_wepoll
 */

namespace test_wepoll_config {

namespace srv {
namespace listen {
constexpr char     *ip     = "0.0.0.0";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace listen
} // namespace srv

namespace cli {

namespace connect {
constexpr char     *ip     = "127.0.0.1";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace connect

} // namespace cli

struct test_s {
    std::function<void(int)> f;
    int                      n;
};

inline auto r_cb = [](socket_t sock, test_s *data) -> void {
    cout << "in" << sock << endl;

    EndPointInfo en;
    int          res = Accept(sock, en);

    if(res == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "Accept ok" << endl;
    }

    auto [ip, port] = EndPointInfo::Dump(en);
    cout << "client : ip " << ip << " port:" << port << endl;

    auto t = (test_s *)data;
    t->f(t->n);
};


inline void server_thread(std::shared_ptr<Epoll<test_s>> ep) {
    using namespace srv;

    auto sock = MakeSocket(listen::family, listen::protol);
    SetSocketReuseAddr(sock);
    SetSocketReusePort(sock);

    EndPointInfo lis;
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
    auto   handler      = make_shared<EventHandle<test_s>::EventHandler>();
    handler->socket_    = sock;
    handler->user_data_ = &i;
    handler->handle_    = ep;
    handler->SetEvents(HandlerEventType::EV_IN);
    handler->Enable();

    ep->Loop();
}

inline void client_thread() {
    using namespace cli;

    EndPointInfo cli_ed;
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

std::shared_ptr<Epoll<test_s>> ep_;

void handle_pipe(int signal) {
    cout << "signal" << endl;
    ep_->Stop();
}

} // namespace test_wepoll_config


inline void test_wepoll() {
    using namespace test_wepoll_config;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    signal(SIGINT, handle_pipe);  // 自定义处理函数

    cout << "test wepoll " << endl;
    auto ep = std::make_shared<Epoll<test_s>>();

    ep_ = ep;
    ep->Init();
    ep->read_ = r_cb;

    thread sr(server_thread, ep);
    thread cl(client_thread);

    sr.join();
    cl.join();
}

#endif