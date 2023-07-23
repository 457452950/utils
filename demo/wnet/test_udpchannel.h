#ifndef UTILS_DEMO_WNET_TEST_UDPCHANNEL_H
#define UTILS_DEMO_WNET_TEST_UDPCHANNEL_H

#include <iostream>

#include "wutils/network/Network.h"

using namespace std;
using namespace wutils::network;


/**
 * test_udpchannel
 */

namespace test_udpchannel_config {

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
// struct udpSession : public UDPChannel::Listener {
//
//    void OnMessage(const uint8_t *message, uint64_t message_len) override {
//        auto [lip, lport] = NetAddress::Dump(srv_ed);
//        auto [rip, rport] = NetAddress::Dump(cli_ed);
//
//        fprintf(stdout,
//                "remote[%s:%d] --> local[%s:%d] msg:[%s] len:%ld\n",
//                rip.c_str(),
//                rport,
//                lip.c_str(),
//                lport,
//                (char *)message,
//                message_len);
//        std::cout.flush();
//
//        udp_chl->Send(message, message_len);
//    };
//    void OnError(wutils::SystemError error) override { cout << "[test_udp]onerr err : " << error << endl; }
//
//    NetAddress                    srv_ed;
//    NetAddress                    cli_ed;
//    std::unique_ptr<UDPChannel> udp_chl;
//};
//
// std::shared_ptr<Epoll<IOEvent>> ep_;
//
// void server_thread() {
//    using namespace srv;
//
//    auto ep = std::make_shared<Epoll<IOEvent>>();
//    ep_     = ep;
//    ep->Init();
//    ep->read_  = in_cb;
//    ep->write_ = out_cb;
//
//    NetAddress srv_ed;
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
//    auto udp_srv = std::make_unique<UDPChannel>(ep);
//    udp_srv->Start(srv_ed, cli_ed, true);
//
//    std::shared_ptr<udpSession> sess = std::make_shared<udpSession>();
//    sess->cli_ed                     = cli_ed;
//    sess->srv_ed                     = srv_ed;
//    udp_srv->SetListener(sess);
//    sess->udp_chl = std::move(udp_srv);
//
//    ep->Loop();
//}
//
// void client_thread() {
//    using namespace wutils;
//    using namespace cli;
//
//    NetAddress cli_ed;
//    // cli_ed.Assign("::1", 4001, AF_FAMILY::INET6);
//    cli_ed.Assign(bind::ip, bind::port, bind::family);
//    auto cli = MakeBindedSocket(cli_ed, true);
//    if(cli == -1) {
//        return;
//    }
//
//    NetAddress srv_ed;
//    if(!srv_ed.Assign(send::ip, send::port, send::family)) {
//        return;
//    }
//
//    char send_msg[] = "afsafsfsfagrtgtbgfbstrbsrbrtbrbstrgbtrbsfdsvbfsdsvbsrtbv";
//
//    NetAddress     srv_;
//    char         cli_buf[1500] = {0};
//    int32_t      len           = 0;
//
//    len = sendto(cli, send_msg, strlen(send_msg), 0, srv_ed.GetSockAddr(), srv_ed.GetSockSize());
//    if(len <= 0) {
//        std::cout << "cli sendto err " << SystemError::GetSysErrCode() << endl;
//    } else {
//        auto [ip, port] = NetAddress::Dump(srv_ed);
//        cout << "cli sendto [" << ip << " : " << port << "] " << std::string(send_msg, strlen(send_msg)).c_str()
//             << endl;
//    }
//
//
//    len = RecvFrom(cli, (uint8_t *)cli_buf, 1500, srv_);
//    if(len <= 0) {
//        std::cout << "cli recv from err " << SystemError::GetSysErrCode() << endl;
//    } else {
//        auto [ip, port] = NetAddress::Dump(srv_);
//        cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() << endl;
//    }
//
//    sendto(cli, send_msg, strlen(send_msg), 0, srv_ed.GetSockAddr(), srv_ed.GetSockSize());
//
//    len = RecvFrom(cli, (uint8_t *)cli_buf, 1500, srv_);
//    if(len <= 0) {
//        std::cout << "cli recv from err " << SystemError::GetSysErrCode() << endl;
//    } else {
//        auto [ip, port] = NetAddress::Dump(srv_);
//        cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() << endl;
//    }
//
//    ::close(cli);
//}
//
//
// void handle_pipe(int signal) {
//    cout << "signal" << endl;
//    ep_->Stop();
//}

} // namespace test_udpchannel_config


void test_udpchannel() {
    using namespace test_udpchannel_config;

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