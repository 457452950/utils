#ifndef UTILS_DEMO_WNET_TEST_UDP_H
#define UTILS_DEMO_WNET_TEST_UDP_H

#include <iostream>

#include "wutils/network/Network.h"

using namespace std;
using namespace wutils::network;


/**
 * test_udp
 */
namespace test_udp_config {

namespace srv {
namespace bind {
constexpr char     *ip     = "192.168.101.2";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
constexpr AF_PROTOL protol = AF_PROTOL::UDP;
} // namespace bind
namespace send {
constexpr char     *ip     = "192.168.101.2";
constexpr int       port   = 4001;
constexpr AF_FAMILY family = AF_FAMILY::INET;
constexpr AF_PROTOL protol = AF_PROTOL::UDP;
} // namespace send
} // namespace srv

namespace cli {

namespace bind {
constexpr char     *ip     = "192.168.101.2";
constexpr int       port   = 4001;
constexpr AF_FAMILY family = AF_FAMILY::INET;
constexpr AF_PROTOL protol = AF_PROTOL::UDP;
} // namespace bind

namespace send {
constexpr char     *ip     = "192.168.101.2";
constexpr int       port   = 4000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
constexpr AF_PROTOL protol = AF_PROTOL::UDP;
} // namespace send

} // namespace cli
//
// std::shared_ptr<Epoll<IOEvent>> ep_;
//
// void server_thread() {
//    using namespace wutils;
//    using namespace srv;
//
//    auto ep = std::make_shared<Epoll<IOEvent>>();
//    ep_     = ep;
//    ep->Init();
//    ep->read_  = in_cb;
//    ep->write_ = out_cb;
//
//    NetAddress srv_ed;
//    // srv_ed.Assign("::1", 4000, AF_FAMILY::INET6);
//    if(!srv_ed.Assign(bind::ip, bind::port, bind::family)) {
//        return;
//    }
//
//    auto [ip, port] = NetAddress::Dump(srv_ed);
//    cout << "[" << ip << ":" << port << "]" << std::endl;
//
//    NetAddress cli_ed;
//    // cli_ed.Assign("::1", 4001, AF_FAMILY::INET6);
//    if(!cli_ed.Assign(send::ip, send::port, send::family)) {
//        return;
//    }
//
//    auto udp_srv = std::make_shared<UDPPointer>(ep);
//    if(!udp_srv->Start(srv_ed, true)) {
//        return;
//    }
//
//    auto onmsg = [&](const wutils::network::NetAddress &local,
//                     const wutils::network::NetAddress &remote,
//                     const uint8_t                   *msg,
//                     uint32_t                         msg_len) {
//        auto [lip, lport] = NetAddress::Dump(local);
//        auto [rip, rport] = NetAddress::Dump(remote);
//
//        fprintf(stdout,
//                "remote[%s:%d] --> local[%s:%d] msg:[%s] len:%d\n",
//                rip.c_str(),
//                rport,
//                lip.c_str(),
//                lport,
//                (char *)msg,
//                msg_len);
//        std::cout.flush();
//
//        udp_srv->SendTo(msg, msg_len, cli_ed);
//    };
//    auto onerr = [](SystemError error) { cout << "[test_udp]onerr err : " << error << endl; };
//
//    udp_srv->OnMessage = onmsg;
//    udp_srv->OnError   = onerr;
//
//    std::thread th1([&]() { ep->Loop(); });
//
//
//    th1.join();
//}
//
// void client_thread() {
//    using namespace cli;
//    using namespace wutils;
//
//    NetAddress cli_ed;
//    // cli_ed.Assign("::1", 4001, AF_FAMILY::INET6);
//    if(!cli_ed.Assign(bind::ip, bind::port, bind::family)) {
//        return;
//    }
//
//    auto cli = MakeBindedSocket(cli_ed);
//    if(cli == -1) {
//        return;
//    }
//
//    NetAddress srv_ed;
//    // srv_ed.Assign("::1", 4000, AF_FAMILY::INET6);
//    if(!srv_ed.Assign(send::ip, send::port, send::family)) {
//        return;
//    }
//
//    std::thread th2([&]() {
//        char send_msg[] = "afsafsfsfagrtgtbgfbstrbsrbrtbrbstrgbtrbsfdsvbfsdsvbsrtbv";
//
//        NetAddress srv_;
//        char     cli_buf[1500] = {0};
//        int32_t  len           = 0;
//
//        sendto(cli, send_msg, strlen(send_msg), 0, srv_ed.GetSockAddr(), srv_ed.GetSockSize());
//
//        len = RecvFrom(cli, (uint8_t *)cli_buf, 1500, srv_);
//        if(len <= 0) {
//            std::cout << "cli recv from err " << SystemError::GetSysErrCode() << endl;
//        } else {
//            auto [ip, port] = NetAddress::Dump(srv_);
//            cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() << endl;
//        }
//
//        sendto(cli, send_msg, strlen(send_msg), 0, srv_ed.GetSockAddr(), srv_ed.GetSockSize());
//
//        len = RecvFrom(cli, (uint8_t *)cli_buf, 1500, srv_);
//        if(len <= 0) {
//            std::cout << "cli recv from err " << SystemError::GetSysErrCode() << endl;
//        } else {
//            auto [ip, port] = NetAddress::Dump(srv_);
//            cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() << endl;
//        }
//    });
//
//    th2.join();
//    ::close(cli);
//}
//
//
// void handle_pipe(int signal) {
//    cout << "signal" << endl;
//    ep_->Stop();
//}

} // namespace test_udp_config

inline void test_udp() {
    using namespace test_udp_config;

    //    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    //    signal(SIGINT, handle_pipe);  // 自定义处理函数
    //
    //    thread sr(server_thread);
    //    thread cl(client_thread);
    //
    //    sr.join();
    //    cl.join();
}


#endif
