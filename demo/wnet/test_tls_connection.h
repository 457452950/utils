#ifndef UTILS_DEMO_WNET_TEST_TLS_CHANNEL_H
#define UTILS_DEMO_WNET_TEST_TLS_CHANNEL_H

#include <csignal>
#include <iostream>
#include <utility>

#include "wutils/network/TlsConnection.h"

using namespace std;
using namespace wutils::network;


// #define CONTEXT event::EpollContext
#define CONTEXT event::SelectContext

/**
 * test_tcp_echo
 */
namespace test_tls_connection_config {

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

class TestSession : public TlsConnection::Listener {
public:
    explicit TestSession(std::shared_ptr<TlsConnection> ch_) : ch(std::move(ch_)) {
        ch->listener_ = this;
        auto err      = ch->Init();
        if(err) {
            LOG(LERROR, "session") << err.message();
            abort();
        }
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
    std::shared_ptr<TlsConnection> ch;
};

shared_ptr<ssl::SslContext>       ssl_context_server;
shared_ptr<ssl::SslContext>       ssl_context_client;
std::shared_ptr<TestSession>      se;
std::shared_ptr<event::IOContext> ep_;

inline auto ac_cb = [](const NetAddress &local, const NetAddress &remote, shared_ptr<event::IOHandle> handler) {
    auto info = remote.Dump();

    LOG(LINFO, "accept") << "accept : info " << std::get<0>(info) << " " << std::get<1>(info);
    auto ch = TlsConnection::Create(local, remote, std::move(handler), ssl_context_server);
    se      = std::make_shared<TestSession>(ch);
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
    using namespace wutils;
    using namespace cli;
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);

    NetAddress cli_ed;
    assert(cli_ed.Assign(connect::ip, connect::port, connect::family));

    tcp::Socket client;
    if(!client.Open(cli_ed.GetFamily())) {
        LOG(LERROR, "client") << "client open fail." << GetGenericError().message();
        return;
    }
    DEFER([&]() { client.Close(); });
    if(!client.Connect(cli_ed)) {
        LOG(LERROR, "client") << "client connect fail." << GetGenericError().message();
        return;
    }

    auto ssl_cli = ssl_context_client->NewSocket(client);
    DEFER([&]() { ssl_cli.Close(); });

    auto err = ssl_cli.Open();
    if(err) {
        LOG(LERROR, "client") << "ssl socket open error : " << err.message();
        return;
    } else {
        LOG(LINFO, "client") << "ssl socket open success : " << err.message();
    }

    //    err = ssl_cli.SslConnect();
    //    if(err) {
    //        LOG(LERROR, "client") << "ssl socket ssl connect error : " << err.message();
    //        return;
    //    } else {
    //        LOG(LINFO, "client") << "ssl socket ssl connect success : " << err.message();
    //    }
    err = ssl_cli.SslDoHandShake();
    if(err) {
        LOG(LERROR, "client") << "ssl socket ssl connect error : " << err.message();
        return;
    } else {
        LOG(LINFO, "client") << "ssl socket ssl connect success : " << err.message();
    }

    auto res = ssl_cli.SslVerifyResult();
    //    auto res = verify_ok;
    if(!res) {
        LOG(LERROR, "client") << "verify res. error : " << X509_verify_cert_error_string(res);
        return;
    } else {
        LOG(LINFO, "client") << "ssl verify pass.  success : " << err.message();
    }

    std::thread thr1([&ssl_cli]() {
        // ::send(cli, "hello", 5, 0);
        int  total = 0;
        char recv_buf[1024];
        while(true) {
            auto [len, err] = ssl_cli.SslRead(recv_buf, size(recv_buf));
            if(err) {
                LOG(LERROR, "client") << "ssl socket ssl read error : " << err.message();
                return;
            } else {
                LOG(LINFO, "client") << "ssl socket ssl read success : " << err.value() << " " << err.message() << " "
                                     << recv_buf;
            }
            total += len;
            // clang-format off
            cout
                << "cli recv :" << std::string(recv_buf, len)
                << " total : " << total
                << endl;
            // clang-format on
            if(total == 2890000) {
                LOG(LINFO, "client") << "stop.";
                ep_->Stop();
                return;
            }
        }
    });

    while(true) {
        static int i = 0;

        char send_buf[] = "hello123hello123hello123hello123hello123hello123hello123hello123hello123"
                          "hello123hello123hello123hello123hello123hello123hello123hello123hello123"
                          "hello123hello123hello123hello123hello123hello123hello123hello123hello123"
                          "hello123hello123hello123hello123hello123hello123hello123hello123hello123";

        err = ssl_cli.SslWrite(send_buf, std::size(send_buf));
        if(err) {
            LOG(LERROR, "client") << "ssl socket ssl send error : " << err.message();
            return;
        } else {
            LOG(LINFO, "client") << "ssl socket ssl send success : " << err.message();
        }
        ++i;
        if(i == 10000) {
            LOG(LINFO, "client") << "send stop";
            break;
        }
        std::this_thread::sleep_for(2ms);
    }

    thr1.join();
    LOG(LINFO, "client") << "client thread end";
}


void handle_pipe(int signal) {
    LOG(LINFO, "signal") << "signal " << signal;
    ep_->Stop();
    se.reset();
}


} // namespace test_tls_connection_config


inline void test_tls_connection() {
    using namespace test_tls_connection_config;
    cout << "-------------------- test tls channel --------------------" << endl;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    signal(SIGINT, handle_pipe);  // 自定义处理函数

    ssl::InitSsl();

    ssl_context_server = ssl::SslContext::Create(ssl::SslContext::TLS, ssl::SslContext::SERVER);
    ssl_context_client = ssl::SslContext::Create(ssl::SslContext::TLS, ssl::SslContext::CLIENT);

    auto err = ssl_context_server->LoadCertificateFile("/tmp/cacert.pem", ssl::SslContext::PEM);
    if(err) {
        LOG(LERROR, "server") << "load file error : " << err.message();
        return;
    } else
        LOG(LINFO, "server") << "load file success : " << err.message();

    err = ssl_context_server->LoadPrivateKey("/tmp/privkey.pem", ssl::SslContext::PEM);
    if(err) {
        LOG(LERROR, "server") << "load file error : " << err.message();
        return;
    } else
        LOG(LINFO, "server") << "load file success : " << err.message();

    err = ssl_context_server->CheckPrivateKey();
    if(err) {
        LOG(LERROR, "server") << "check private key error : " << err.message();
        return;
    } else {
        LOG(LINFO, "server") << "check private key success : " << err.message();
    }

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

    ssl::ReleaseSsl();

    cout << "-------------------- test tls channel end --------------------" << endl;
}


#endif // UTILS_DEMO_WNET_TEST_TLS_CHANNEL_H