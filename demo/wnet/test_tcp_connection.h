#ifndef UTILS_DEMO_WNET_TEST_TCPCHANNEL_H
#define UTILS_DEMO_WNET_TEST_TCPCHANNEL_H

#include <csignal>
#include <iostream>
#include <utility>

#include "wutils/network/Network.h"

using namespace std;
using namespace wutils::network;


#define CONTEXT event::EpollContext
// #define CONTEXT event::SelectContext

/**
 * test_tcp_echo
 */
namespace test_tcp_connection_config {

namespace srv {
namespace listen {
// constexpr char     *ip     = "0:0:0:0:0:0:0:0";
// constexpr int       port   = 4000;
// constexpr AF_FAMILY family = AF_FAMILY::INET6;
constexpr char     *ip     = "0.0.0.0";
constexpr int       port   = 12000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
} // namespace listen
} // namespace srv

namespace cli {

namespace connect {
// constexpr char     *ip     = "0:0:0:0:0:0:0:0";
// constexpr int       port   = 4000;
// constexpr AF_FAMILY family = AF_FAMILY::INET6;
constexpr char     *ip     = "0.0.0.0";
constexpr int       port   = 12000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
} // namespace connect

} // namespace cli

class TestSession : public Connection::Listener {
public:
    explicit TestSession(std::shared_ptr<Connection> ch_) : ch(std::move(ch_)) {
        ch->listener_ = this;
        assert(!ch->Init());
    }

    void OnDisconnect() override {
        LOG(LERROR, "session") << "disconnected";
        ch->listener_ = nullptr;
        this->ch.reset();
    }
    void OnReceive(Data data) override {
        //        cout << "recv " << std::string((char *)message, (int)message_len) << " size " << message_len << endl;
        ch->Send(data.data, data.bytes);
    }
    void OnError(wutils::Error error) override {
        LOG(LERROR, "session") << error;
        ch->listener_ = nullptr;
        this->ch.reset();
    }

    // private:
    std::shared_ptr<Connection> ch;
};

std::shared_ptr<TestSession>      se;
std::shared_ptr<event::IOContext> ep_;

inline auto ac_cb = [](const NetAddress &local, const NetAddress &remote, unique_ptr<event::IOHandle> handler) {
    auto info = remote.Dump();

    LOG(LINFO, "accept") << "accept : info " << std::get<0>(info) << " " << std::get<1>(info);
    auto ch = Connection::Create(local, remote, std::move(handler));
    se      = make_shared<TestSession>(ch);
    assert(!ch->Init());
};
inline auto err_cb = [](wutils::Error error) { std::cout << error << std::endl; };


void server_thread() {
    using namespace srv;

    auto ep = CONTEXT::Create();
    ep->Init();
    ep_ = ep;

    NetAddress local_ed;
    if(!local_ed.Assign(listen::ip, listen::port, listen::family)) {
        return;
    }

    auto accp_channel = Acceptor::Create(ep);

    if(!accp_channel->Open(local_ed.GetFamily())) {
        LOG(LERROR, "server") << wutils::GetGenericError();
        abort();
    }

    accp_channel->OnAccept = std::bind(ac_cb, local_ed, std::placeholders::_1, std::placeholders::_2);
    accp_channel->OnError  = err_cb;

    if(accp_channel->Start(local_ed)) {
        LOG(LERROR, "server") << wutils::GetGenericError().message();
        abort();
    }

    auto info = accp_channel->GetLocal().Dump();
    LOG(LINFO, "server") << "start ok." << std::get<0>(info) << " " << std::get<1>(info);

    ep_->Loop();
    cout << wutils::GetGenericError().message() << endl;

    LOG(LINFO, "server") << "server thread end";
    // 激活客户端的 阻塞recv
}

void client_thread() {
    using namespace cli;
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);

    NetAddress cli_ed;
    assert(cli_ed.Assign(connect::ip, connect::port, connect::family));

    tcp::Socket cli;
    cli.Open(connect::family);
    bool res = cli.Connect(cli_ed);

    DEFER([&cli]() { cli.Close(); });

    if(!res) {
        LOG(LERROR, "client") << "connect fail, err : " << wutils::GetGenericError().message();
        return;
    } else {
        LOG(LINFO, "client") << "connect ok";
        cli.Send((uint8_t *)"123123", 6);
    }

    std::atomic_int count{0};

    std::thread thr1([&cli, &count]() {
        // ::send(cli, "hello", 5, 0);
        int  total = 0;
        char buf[1500];
        while(true) {
            ep_->Post([&]() {
                count.fetch_add(1);
                //                LOG(LINFO, "post") << "hello word.";
            });
            auto l = cli.Recv((uint8_t *)buf, 1500);
            if(l == 0) {
                LOG(LINFO, "client") << "dis connected.";
                break;
            }
            total += l;
            // clang-format off
//            cout
//                << "cli recv :" << std::string(buf, l)
//                << " total : " << total
//                << endl;
            // clang-format on
            if(total == 2190006) {
                LOG(LINFO, "client") << "stop.";
                ep_->Stop();
                return;
            }
        }
    });

    while(true) {
        static int i = 0;

        cli.Send((uint8_t *)"hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73);
        cli.Send((uint8_t *)"hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73);
        cli.Send((uint8_t *)"hello123hello123hello123hello123hello123hello123hello123hello123hello123", 73);
        ++i;
        if(i == 10000) {
            LOG(LINFO, "client") << "send stop";
            break;
        }
        std::this_thread::sleep_for(2ms);
    }

    thr1.join();
    LOG(LINFO, "client") << "client thread end " << count.load();
}


void handle_pipe(int signal) {
    LOG(LINFO, "signal") << "signal " << signal;
    ep_->Stop();
    se.reset();
}


} // namespace test_tcp_connection_config


inline void test_tcp_connection() {
    using namespace test_tcp_connection_config;
    cout << "-------------------- test tcp channel --------------------" << endl;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    signal(SIGINT, handle_pipe);  // 自定义处理函数

    thread sr(server_thread);
    thread cl(client_thread);

    if(sr.joinable())
        sr.join();
    if(cl.joinable())
        cl.join();

    if(ep_)
        ep_.reset();
    if(se)
        se.reset();
    cout << "-------------------- test tcp channel end --------------------" << endl;
}


#endif