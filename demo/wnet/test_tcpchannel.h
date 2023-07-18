#ifndef UTILS_DEMO_WNET_TEST_TCPCHANNEL_H
#define UTILS_DEMO_WNET_TEST_TCPCHANNEL_H

#include <iostream>
#include <utility>

#include "wutils/network/NetWork.h"

using namespace std;
using namespace wutils::network;


/**
 * test_tcpchannel
 */
namespace test_tcpchannel_config {

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

class TestSession : public Connection::Listener {
public:
    TestSession(std::shared_ptr<Connection> ch_) : ch(std::move(ch_)) { ch->listener_ = this; }

    void OnDisconnect() override {
        std::cout << "disconnect" << std::endl;
        this->ch.reset();
    }
    void OnReceive(Data data) override {
        //        cout << "recv " << std::string((char *)message, (int)message_len) << " size " << message_len << endl;
        ch->Send(data.data, data.bytes);
    }
    void OnError(wutils::SystemError error) override { std::cout << error << endl; }

    // private:
    std::shared_ptr<Connection> ch;
};

std::shared_ptr<TestSession>         se;
std::atomic_bool                     active{true};
std::shared_ptr<event::EpollContext> ep_;

inline auto ac_cb = [](const EndPoint &local, const EndPoint &remote, unique_ptr<event::IOHandle> handler) {
    auto info = EndPoint::Dump(remote);

    cout << "recv : info " << std::get<0>(info) << " " << std::get<1>(info) << std::endl;
    auto ch = std::make_shared<Connection>(local, remote, std::move(handler));
    se      = std::make_shared<TestSession>(ch);
};


void server_thread() {
    using namespace srv;

    auto ep = std::make_shared<event::EpollContext>();
    ep->Init();
    ep_ = ep;

    EndPoint local_ed;
    if(!local_ed.Assign(listen::ip, listen::port, listen::family)) {
        return;
    }

    auto accp_channel = new Acceptor(ep);
    accp_channel->Start(local_ed);
    accp_channel->OnAccept = ac_cb;

    ep->Loop();
    cout << wutils::SystemError::GetSysErrCode() << endl;

    // 激活客户端的 阻塞recv
    //    se->ch->Send((uint8_t *)"s", 1);
    delete accp_channel;
}

void client_thread() {
    using namespace cli;

    EndPoint cli_ed;
    if(!cli_ed.Assign(connect::ip, connect::port, connect::family)) {
        return;
    }
    auto cli = MakeSocket(connect::family, connect::protol);
    bool res = ConnectToHost(cli, cli_ed);

    if(!res) {
        cout << "[test_tcpchannel]connect error : " << strerror(errno) << endl;
        ::close(cli);
        return;
    } else {
        cout << "connect ok" << endl;
        ::send(cli, "123123", 6, 0);
    }

    std::thread thr1([cli]() {
        // ::send(cli, "hello", 5, 0);
        int  total = 0;
        char buf[1500];
        while(active) {
            auto l = ::recv(cli, buf, 1500, 0);
            if(l == 0) {
                break;
            }
            total += l;
            // clang-format off
//            cout
//                 << "cli recv :" << std::string(buf, l)
//                << " total : " << total
//                << endl;
            // clang-format on
        }
    });

    using namespace std::chrono;

    Timer t(ep_);
    t.OnTime = [cli, &t]() {
        // cout << std::chrono::duration_cast<std::chrono::milliseconds>(
        //                 std::chrono::system_clock::now().time_since_epoch())
        //                 .count()
        //      << " ontime!!!" << endl;
        static int i = 0;

        ::send(cli, "hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73, 0);
        ::send(cli, "hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73, 0);
        ::send(cli, "hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73, 0);
        ++i;
        if(i == 10000)
            t.Stop();
    };
    t.Start(100ms, 1ms);

    thr1.join();
    ::close(cli);
}


void handle_pipe(int signal) {
    cout << "signal" << endl;
    ep_->Stop();
    active.store(false);
}


} // namespace test_tcpchannel_config


inline void test_tcpchannel() {
    using namespace test_tcpchannel_config;
    cout << "test channel " << endl;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    signal(SIGINT, handle_pipe);  // 自定义处理函数

    thread sr(server_thread);
    thread cl(client_thread);

    sr.join();
    cl.join();
}


#endif