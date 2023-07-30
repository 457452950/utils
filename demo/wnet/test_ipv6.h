#ifndef UTILS_DEMO_WNET_TEST_IPV6_H
#define UTILS_DEMO_WNET_TEST_IPV6_H


#include <iostream>
#include <thread>

#include "wutils/network/Network.h"
#include "wutils/base/Defer.h"

using namespace std;
using namespace wutils::network;


/**
 * test_ipv6
 */

namespace test_ipv6_config {

namespace srv {
namespace listen {
constexpr char     *ip     = "0:0:0:0:0:0:0:0";
constexpr int       port   = 12000;
constexpr AF_FAMILY family = AF_FAMILY::INET6;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace listen
} // namespace srv

namespace cli {

namespace connect {
constexpr char     *ip     = "0:0:0:0:0:0:0:0";
constexpr int       port   = 12000;
constexpr AF_FAMILY family = AF_FAMILY::INET6;
constexpr AF_PROTOL protol = AF_PROTOL::TCP;
} // namespace connect

} // namespace cli

void server_thread() {
    // bind and listen

    NetAddress srv_ed;
    if(!srv_ed.Assign(srv::listen::ip, srv::listen::port, srv::listen::family)) {
        return;
    }

    auto sock = MakeSocket(srv::listen::family, srv::listen::protol);
    if(sock == -1) {
        return;
    }

    DEFER([sock]() { ::close(sock); });

    // int  res  = Bind(sock, local_ip, 4000, 0);
    int res = Bind(sock, srv_ed);
    if(!res) {
        cout << "bind error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "bind ok" << endl;
    }

    res = listen(sock, 5);
    if(res == -1) {
        cout << "listen error : " << errno << endl;
    } else {
        cout << "listen ok" << endl;
    }

    // accept

    NetAddress en;
    res = Accept(sock, en);

    if(res == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "Accept ok" << endl;
    }

    DEFER([res]() { ::close(res); });

    auto [ip, port] = en.Dump();
    cout << "client : ip " << ip << " port:" << port << endl;
}

void client_thread() {
    NetAddress cli_ed;
    if(!cli_ed.Assign(cli::connect::ip, cli::connect::port, cli::connect::family)) {
        return;
    }
    auto cli = MakeSocket(cli::connect::family, cli::connect::protol);
    if(cli == -1) {
        return;
    }

    DEFER([cli]() { ::close(cli); });

    auto res = ConnectToHost(cli, cli_ed);
    if(!res) {
        cout << "[test_ipv6]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
    }
}

} // namespace test_ipv6_config

inline void test_ipv6() {
    using namespace test_ipv6_config;

    thread sr(server_thread);
    thread cl(client_thread);

    sr.join();
    cl.join();
}


#endif // UTILS_DEMO_WNET_TEST_IPV6_H