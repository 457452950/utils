#ifndef UTILS_DEMO_WNET_TEST_UDPCHANNEL_H
#define UTILS_DEMO_WNET_TEST_UDPCHANNEL_H

#include <iostream>

#include "WNetWork/WNetWork.h"

using namespace std;
using namespace wlb::network;


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
struct udpSession : public WUDPChannel::Listener {

    virtual void OnMessage(const uint8_t *message, uint64_t message_len) {
        auto [lip, lport] = WEndPointInfo::Dump(srv_ed);
        auto [rip, rport] = WEndPointInfo::Dump(cli_ed);

        fprintf(stdout,
                "remote[%s:%d] --> local[%s:%d] msg:[%s] len:%ld\n",
                rip.c_str(),
                rport,
                lip.c_str(),
                lport,
                (char *)message,
                message_len);
        std::cout.flush();

        udp_chl->Send(message, message_len);
    };
    virtual void OnError(const char *err_message) { cout << "[test_udp]onerr err : " << err_message << endl; }

    WEndPointInfo                srv_ed;
    WEndPointInfo                cli_ed;
    std::unique_ptr<WUDPChannel> udp_chl;
};

std::shared_ptr<WEpoll<WBaseChannel>> ep_;

void server_thread() {
    using namespace srv;

    auto ep = std::make_shared<WEpoll<WBaseChannel>>();
    ep_     = ep;
    ep->Init();
    ep->read_  = in_cb;
    ep->write_ = out_cb;

    WEndPointInfo srv_ed;
    if(!srv_ed.Assign(bind::ip, bind::port, bind::family)) {
        return;
    }

    auto [ip, port] = WEndPointInfo::Dump(srv_ed);
    cout << "[" << ip << ":" << port << "]" << std::endl;

    WEndPointInfo cli_ed;
    // cli_ed.Assign("::1", 4001, AF_FAMILY::INET6);
    if(!cli_ed.Assign(send::ip, send::port, send::family)) {
        return;
    }

    auto udp_srv = std::make_unique<WUDPChannel>(ep);
    udp_srv->Start(srv_ed, cli_ed, true);

    std::shared_ptr<udpSession> sess = std::make_shared<udpSession>();
    sess->cli_ed                     = cli_ed;
    sess->srv_ed                     = srv_ed;
    udp_srv->SetListener(sess);
    sess->udp_chl = std::move(udp_srv);

    ep->Loop();
}

void client_thread() {
    using namespace cli;

    WEndPointInfo cli_ed;
    // cli_ed.Assign("::1", 4001, AF_FAMILY::INET6);
    cli_ed.Assign(bind::ip, bind::port, bind::family);
    auto cli = MakeBindedSocket(cli_ed, true);
    if(cli == -1) {
        return;
    }

    WEndPointInfo srv_ed;
    if(!srv_ed.Assign(send::ip, send::port, send::family)) {
        return;
    }

    char send_msg[] = "afsafsfsfagrtgtbgfbstrbsrbrtbrbstrgbtrbsfdsvbfsdsvbsrtbv";

    WEndPointInfo srv_;
    char          cli_buf[1500] = {0};
    int32_t       len           = 0;

    len = sendto(cli, send_msg, strlen(send_msg), 0, srv_ed.GetAddr(), srv_ed.GetSockSize());
    if(len <= 0) {
        std::cout << "cli sendto err " << ErrorToString(GetError()) << endl;
    } else {
        auto [ip, port] = WEndPointInfo::Dump(srv_ed);
        cout << "cli sendto [" << ip << " : " << port << "] " << std::string(send_msg, strlen(send_msg)).c_str()
             << endl;
    }


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

    ::close(cli);
}


void handle_pipe(int signal) {
    cout << "signal" << endl;
    ep_->Stop();
}

} // namespace test_udpchannel_config


void test_udpchannel() {
    using namespace test_udpchannel_config;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    signal(SIGINT, handle_pipe);  // 自定义处理函数

    thread sr(server_thread);
    thread cl(client_thread);

    sr.join();
    cl.join();
}


#endif