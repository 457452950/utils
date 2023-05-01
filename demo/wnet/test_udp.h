#ifndef UTILS_DEMO_WNET_TEST_UDP_H
#define UTILS_DEMO_WNET_TEST_UDP_H

#include <iostream>

#include "WNetWork/WNetWork.h"

using namespace std;
using namespace wlb::network;


/**
 * test_udp
 */
namespace test_udp_config {

inline auto in_cb = [](socket_t sock, WBaseChannel *data) {
    auto *ch = (ReadChannel *)data;
    // cout << "get channel call channel in" << std::endl;
    ch->ChannelIn();
};
inline auto out_cb = [](socket_t sock, WBaseChannel *data) {
    auto *ch = (WriteChannel *)data;
    // cout << "get channel call channel in" << std::endl;
    ch->ChannelOut();
};

} // namespace test_udp

inline void test_udp() {
    using namespace test_udp_config;

    auto ep = std::make_shared<WEpoll<WBaseChannel>>();
    ep->Init();
    ep->read_  = in_cb;
    ep->write_ = out_cb;

    WEndPointInfo srv_ed;
    // srv_ed.Assign("::1", 4000, AF_FAMILY::INET6);
    srv_ed.Assign("192.168.101.2", 4000, AF_FAMILY::INET);
    auto [ip, port] = WEndPointInfo::Dump(srv_ed);
    cout << "[" << ip << ":" << port << "]" << std::endl;

    WEndPointInfo cli_ed;
    // cli_ed.Assign("::1", 4001, AF_FAMILY::INET6);
    cli_ed.Assign("192.168.101.2", 4001, AF_FAMILY::INET);
    auto          cli = MakeBindedSocket(cli_ed);
    WEndPointInfo cli2_ed;
    // cli_ed.Assign("::1", 4001, AF_FAMILY::INET6);
    cli2_ed.Assign("192.168.101.2", 4002, AF_FAMILY::INET);
    auto cli2 = MakeBindedSocket(cli2_ed);

    auto udp_srv = new WUDP(ep);
    udp_srv->Start(srv_ed);

    auto onmsg = [&](const wlb::network::WEndPointInfo &local,
                     const wlb::network::WEndPointInfo &remote,
                     const uint8_t                     *msg,
                     uint32_t                           msg_len) {
        auto [lip, lport] = WEndPointInfo::Dump(local);
        auto [rip, rport] = WEndPointInfo::Dump(remote);

        fprintf(stdout,
                "remote[%s:%d] --> local[%s:%d] msg:[%s] len:%d\n",
                rip.c_str(),
                rport,
                lip.c_str(),
                lport,
                (char *)msg,
                msg_len);
        std::cout.flush();

        udp_srv->SendTo(msg, msg_len, cli_ed);
    };
    auto onerr = [](const char *msg) { cout << "[test_udp]onerr err : " << msg << endl; };

    udp_srv->OnMessage = onmsg;
    udp_srv->OnError   = onerr;

    std::thread th1([&]() { ep->Loop(); });

    std::thread th2([&]() {
        char send_msg[] = "afsafsfsfagrtgtbgfbstrbsrbrtbrbstrgbtrbsfdsvbfsdsvbsrtbv";

        WEndPointInfo srv_;
        char          cli_buf[1500] = {0};
        int32_t       len           = 0;

        sendto(cli, send_msg, strlen(send_msg), 0, srv_ed.GetAddr(), srv_ed.GetSockSize());

        len = RecvFrom(cli, (uint8_t *)cli_buf, 1500, srv_);
        if(len <= 0) {
            std::cout << "cli recv from err " << ErrorToString(GetError()) << endl;
        } else {
            auto [ip, port] = WEndPointInfo::Dump(srv_);
            cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() << endl;
        }

        sendto(cli, send_msg, strlen(send_msg), 0, srv_ed.GetAddr(), srv_ed.GetSockSize());

        len = RecvFrom(cli, (uint8_t *)cli_buf, 1500, srv_);
        if(len <= 0) {
            std::cout << "cli recv from err " << ErrorToString(GetError()) << endl;
        } else {
            auto [ip, port] = WEndPointInfo::Dump(srv_);
            cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() << endl;
        }
    });
    th1.join();
}


#endif
