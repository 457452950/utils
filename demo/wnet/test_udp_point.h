#ifndef UTILS_DEMO_WNET_TEST_UDP_H
#define UTILS_DEMO_WNET_TEST_UDP_H

#include <iostream>
#include <string>
#include <utility>

#include "wutils/network/Network.h"

using namespace std;
using namespace wutils::network;

// #define CONTEXT event::EpollContext
#define CONTEXT event::SelectContext

/**
 * test_udp
 */
namespace test_udp_config {

namespace srv {
namespace bind {
constexpr char const *ip     = v4::ANY;
constexpr int         port   = 4000;
constexpr AF_FAMILY   family = AF_FAMILY::INET;
constexpr AF_PROTOL   protol = AF_PROTOL::UDP;
} // namespace bind
} // namespace srv

namespace cli {
namespace bind {
constexpr char const *ip     = v4::LOOPBACK;
constexpr int         port   = 4001;
constexpr AF_FAMILY   family = AF_FAMILY::INET;
constexpr AF_PROTOL   protol = AF_PROTOL::UDP;
} // namespace bind
} // namespace cli

std::shared_ptr<CONTEXT> ep_;

NetAddress server_na;
NetAddress client_na;

class UdpServer : public UdpPoint::Listener {
public:
    explicit UdpServer(shared_ptr<UdpPoint> point) : point_(std::move(point)) { point_->listener_ = this; }
    ~UdpServer() { LOG(LINFO, "server") << "recv total :" << recv_total_.load(); }

    void OnError(wutils::SystemError error) override {
        LOG(LERROR, "server") << error;
        point_->listener_ = nullptr;
        point_.reset();
    }
    void OnReceiveFrom(wutils::network::Package package, wutils::network::NetAddress remote) override {
        //        LOG(LINFO, "server") << "recv pack : " << std::string((char *)package.data, package.bytes).c_str() <<
        //        " len " << package.bytes;

        recv_total_.fetch_add(package.bytes);
        point_->SendTo(package, remote);
    }

    shared_ptr<UdpPoint> point_;
    std::atomic_uint64_t recv_total_{0};
};

void server_thread() {
    using namespace wutils;
    using namespace srv;

    auto ep = CONTEXT::Create();
    ep_     = ep;
    ep->Init();


    auto udp_srv    = std::make_shared<UdpPoint>(ep);
    auto udp_server = make_shared<UdpServer>(udp_srv);

    if(!udp_srv->Open(server_na.GetFamily())) {
        LOG(LERROR, "server") << "open fail." << SystemError::GetSysErrCode();
        return;
    }

    if(!udp_srv->Bind(server_na)) {
        LOG(LERROR, "server") << "bind fail." << SystemError::GetSysErrCode();
        return;
    }
    auto [ip, port] = udp_srv->GetLocalAddress().Dump();
    LOG(LINFO, "server") << "bind ok "
                         << "[" << ip << ":" << port << "]";

    ep->Loop();
    LOG(LINFO, "server") << "server thread end";
}

void client_thread() {
    using namespace cli;
    using namespace wutils;
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);

    udp::Socket cli;
    assert(cli.Open(client_na.GetFamily()));
    assert(cli.Bind(client_na));
    assert(cli.Connect(server_na));

    char send_msg[] = "afsafsfsfagrtgtbgfbstrbsrbrtbrbstrgbtrbsfdsvbfsdsvbsrtbv";

    int i = 1000000;
    while(i--) {
        NetAddress srv_;
        char       cli_buf[1500] = {0};
        int32_t    len           = 0;

        //        cout << "cli send ." << endl;
        len = cli.Send((uint8_t *)send_msg, std::size(send_msg));
        if(len <= 0) {
            //            std::cout << "cli send err " << SystemError::GetSysErrCode() << endl;
            return;
        } else {
            //            cout << "cli send over. " << endl;
        }

        len = cli.RecvFrom((uint8_t *)cli_buf, 1500, &srv_);
        if(len <= 0) {
            //            std::cout << "cli recv from err " << SystemError::GetSysErrCode() << endl;
            return;
        } else {
            auto [ip, port] = srv_.Dump();
            //            cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() <<
            //            endl;
        }
    };

    cli.Close();
    LOG(LINFO, "client") << "client thread end";
    ep_->Stop();
}


void handle_pipe(int signal) {
    cout << "signal" << endl;
    ep_->Stop();
}

} // namespace test_udp_config

inline void test_udp() {
    using namespace test_udp_config;
    cout << "-------------------- test udp point --------------------" << endl;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    signal(SIGINT, handle_pipe);  // 自定义处理函数

    if(!server_na.Assign(srv::bind::ip, srv::bind::port, srv::bind::family)) {
        LOG(LERROR, "test") << "assign server net address fail." << wutils::SystemError::GetSysErrCode();
        return;
    }
    if(!client_na.Assign(cli::bind::ip, cli::bind::port, cli::bind::family)) {
        LOG(LERROR, "test") << "assign client net address fail." << wutils::SystemError::GetSysErrCode();
        return;
    }

    thread sr(server_thread);
    thread cl(client_thread);

    sr.join();
    cl.join();

    if(ep_) {
        ep_.reset();
    }
    cout << "-------------------- test udp point end --------------------" << endl;
}


#endif
