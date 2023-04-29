#include <csignal>
#include <iostream>
#include <memory>

#include <sys/uio.h>

#include "Channel.h"
#include "WDebugger.hpp"
#include "WNetWork/WNetWork.h"
#include "WNetWork/stdIOVec.h"

using namespace std;
using namespace wlb::network;

void handle_pipe(int signal) { cout << "signal" << endl; }

void test_preadv_pwritev();
void test_iovec();
void test_ipv6();
void test_wepoll();
void test_tcpchannel();
void test_udp();
void test_udpchannel();
void test_tcpserver();
void test_myChannel();

// class t {
// public:
//     t() { cout << std::this_thread::get_id() << " t()" << endl; }
//     ~t() { cout << "~t()" << endl; }
//     int i = 1;
// };

// class B {
// public:
//     B() { cout << std::this_thread::get_id() << " B()" << endl; }
//     ~B() { cout << "~B()" << endl; }
//     B(const B &) { cout << std::this_thread::get_id() << " B(const B&)" << endl; }

//     void pr() const { cout << "t : " << t_.i << endl; }

//     static thread_local t t_;
// };
// thread_local t B::t_;

int main() {


    signal(SIGPIPE, handle_pipe); // 自定义处理函数

    cout << sizeof(sockaddr) << " " << sizeof(sockaddr_in) << " " << sizeof(sockaddr_in6) << endl;

    // test_preadv_pwritev();
    // test_iovec();
    // test_ipv6();
    // test_wepoll();

    test_tcpchannel();
    // test_udp();
    // test_udpchannel();
    // test_tcpserver();
    // test_myChannel();
}

/**
 * test_preadv_pwritev
 */
void test_preadv_pwritev() {

    auto sock = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    SetSocketReuseAddr(sock);
    // int  res  = Bind(sock, "0:0:0:0:0:0:0:0", 4000, 0);
    int srv = Bind(sock, *(WEndPointInfo::MakeWEndPointInfo("0:0:0:0:0:0:0:0", 4000, wlb::network::AF_FAMILY::INET6)));

    if(!srv) {
        cout << "bind error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "bind ok" << endl;
    }

    srv = listen(sock, 5);
    if(srv == -1) {
        cout << "listen error : " << errno << endl;
        return;
    } else {
        cout << "listen ok" << endl;
    }

    auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    SetSocketNoBlock(cli);
    bool ok = ConnectToHost(cli, "0:0:0:0:0:0:0:0", 4000, 0);
    cout << "[test_preadv_pwritev]ConnectToHost error : " << strerror(errno) << endl;
    if(!ok) {
        cout << "[test_preadv_pwritev]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
    }

    WEndPointInfo en;
    srv = Accept4(sock, &en, SOCK_NONBLOCK);
    // ok  = SetSocketNoBlock(srv);
    // if(!ok) {
    //     cout << "[test_preadv_pwritev]SetSocketNoBlock error : " << strerror(errno) << endl;
    // } else {
    //     cout << "SetSocketNoBlock ok" << endl;
    // }

    if(srv == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "Accept ok " << srv << endl;
    }

    int  totol = 0;
    auto info  = WEndPointInfo::Dump(en);
    cout << "client : ip " << std::get<0>(info) << " port:" << std::get<1>(info) << endl;

    const char *str0    = "helasdadafsafaafsadfdsaasfasfafafasfaufasnukfgagfasnjknfajkfbjasbfab,"
                          "gkhjbgherabllhjbfahjlbehjrlgbhaejrbgjaelrkbghaerjbghajergberhjgbeshjblo";
    const char *str1    = "1\n";
    const char *str2    = "2\n";
    const char *str3    = "3\n";
    const char *str4    = "4\n";
    const char *str5    = "5\n";
    const char *str6    = "6\n";
    const char *str7    = "7\n";
    const char *str8    = "8\n";
    const char *str9    = "9\n";
    const char *strs[9] = {"1\n", "2\n", "3\n", "4\n", "5\n", "6\n", "7\n", "8\n", "9\n"};
    // char        *str10 = "10\n";
    struct iovec iov[2];
    ssize_t      nwritten;

    iov[0].iov_base = (void *)str0;
    iov[0].iov_len  = strlen(str0);
    iov[1].iov_base = (void *)str1;
    iov[1].iov_len  = strlen(str1);

    // send(srv, str0, strlen(str0), 0);

    for(size_t i = 0; i < 50000; i++) {
        iov[1].iov_base = (void *)strs[i % 9];

        // nwritten = pwritev2(srv, iov, 2, -1, RWF_APPEND);
        nwritten = pwritev2(srv, iov, 2, -1, 0);
        // nwritten        = writev(srv, iov, 2);
        // nwritten = send(srv, str0, strlen(str0), 0);

        totol += nwritten;
        if(nwritten == 142)
            continue;
        cout << "pwritev2 size " << nwritten << endl;
        if(nwritten == -1) {
            auto err = GetError();
            cout << iov[0].iov_len << " " << iov[1].iov_len << endl;
            cout << err << " " << EAGAIN << endl;
            cout << "pwritev2 error " << ErrorToString(err) << endl;
            break;
        }
    }
    cout << "send size totol : " << totol << endl;


    char    recv_buf[150000];
    ssize_t res = ::recv(cli, recv_buf, 150000, 0);
    cout << "'" << recv_buf << "' \n" << strlen(recv_buf) << endl;
    res = ::recv(cli, recv_buf, 150000, 0);
    cout << "recv res : " << res << endl;
    if(res == -1) {
        auto err = GetError();
        cout << err << " " << EWOULDBLOCK << endl;
        cout << "recv error " << ErrorToString(err) << endl;
    }
}

/**
 * test_iovec
 */
void test_iovec() {

    auto sock = MakeSocket(AF_FAMILY::INET, AF_PROTOL::TCP);
    SetSocketReuseAddr(sock);
    // int  res  = Bind(sock, "0:0:0:0:0:0:0:0", 4000, 0);
    int srv = Bind(sock, *(WEndPointInfo::MakeWEndPointInfo("127.0.0.1", 4000, wlb::network::AF_FAMILY::INET)));

    if(!srv) {
        cout << "bind error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "bind ok" << endl;
    }

    srv = listen(sock, 5);
    if(srv == -1) {
        cout << "listen error : " << errno << endl;
        return;
    } else {
        cout << "listen ok" << endl;
    }

    auto cli = MakeSocket(AF_FAMILY::INET, AF_PROTOL::TCP);
    SetSocketNoBlock(cli);
    bool ok = ConnectToHost(cli, "127.0.0.1", 4000, 1);
    cout << "[test_preadv_pwritev]ConnectToHost error : " << strerror(errno) << endl;
    if(!ok) {
        cout << "[test_preadv_pwritev]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
    }

    WEndPointInfo en;
    srv = Accept4(sock, &en, SOCK_NONBLOCK);

    if(srv == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "Accept ok " << srv << endl;
    }

    auto info = WEndPointInfo::Dump(en);
    cout << "client : ip " << std::get<0>(info) << " port:" << std::get<1>(info) << endl;

    for(int i = 0; i < 3; ++i) {
        const char *str0 = "hello this is message 123";
        ssize_t     nwritten;


        // send(srv, str0, strlen(str0), 0);

        for(size_t j = 0; j < 3; j++) {

            nwritten = send(srv, str0, strlen(str0), 0);

            cout << "send size " << nwritten << endl;
            if(nwritten == -1) {
                auto err = GetError();
                cout << "pwritev2 error " << ErrorToString(err) << endl;
                break;
            }
        }

        IOVec   vec1(5, 20);
        IOVec   vec2(5, 20);
        char    recv_buf[1500];
        ssize_t res;

        for(size_t j = 0; j < 2; j++) {
            vec1.Write([&](iovec *v, int l) -> int64_t {
                cout << "can " << l << endl;
                auto res = readv(cli, v, l);
                cout << "readv res : " << res << endl;
                if(res == -1) {
                    auto err = GetError();
                    cout << "readv error " << ErrorToString(err) << endl;
                }
                return res;
            });
        }
        // res = ::recv(cli, recv_buf, 1500, 0);
        // cout << "recv res : " << res << endl;
        // if(res == -1) {
        //     auto err = GetError();
        //     cout << "recv error " << ErrorToString(err) << endl;
        // } else {
        //     recv_buf[res] = '\0';
        //     cout << "recv res : " << recv_buf << endl;
        // }

        // res = ::recv(cli, recv_buf, 150000, 0);
        for(size_t j = 0; j < 1; j++) {
            vec1.Read([&](const iovec *v, int l) -> int64_t {
                auto res = writev(cli, v, l);
                cout << "writev res : " << res << endl;
                if(res == -1) {
                    auto err = GetError();
                    cout << "writev error " << ErrorToString(err) << endl;
                }
                return res;
            });
        }

        res = ::recv(srv, recv_buf, 1500, 0);
        cout << "recv res : " << res << endl;
        if(res == -1) {
            auto err = GetError();
            cout << "recv error " << ErrorToString(err) << endl;
        } else {
            recv_buf[res] = '\0';
            cout << "recv res : " << recv_buf << endl;
        }
    }
    ::close(srv);
    ::close(cli);
    ::close(sock);
}

/**
 * test_ipv6
 */
void test_ipv6() {

    auto sock = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    // int  res  = Bind(sock, "0:0:0:0:0:0:0:0", 4000, 0);
    int res = Bind(sock, *(WEndPointInfo::MakeWEndPointInfo("0:0:0:0:0:0:0:0", 4000, wlb::network::AF_FAMILY::INET6)));

    if(!res) {
        cout << "bind error : " << strerror(errno) << endl;
    } else {
        cout << "bind ok" << endl;
    }

    res = listen(sock, 5);
    if(res == -1) {
        cout << "listen error : " << errno << endl;
    } else {
        cout << "listen ok" << endl;
    }

    auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    res      = ConnectToHost(cli, "0:0:0:0:0:0:0:1", 4000, 0);

    if(!res) {
        cout << "[test_ipv6]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
    }

    WEndPointInfo en;
    res = Accept(sock, &en);

    if(res == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
    } else {
        cout << "Accept ok" << endl;
    }

    auto info = WEndPointInfo::Dump(en);
    cout << "client : ip " << std::get<0>(info) << " port:" << std::get<1>(info) << endl;
}


/**
 * test_wepoll
 */
struct test_s {
    std::function<void(int)> f;
    int                      n;
};

auto r_cb = [](socket_t sock, WSelect<test_s>::user_data_ptr data) {
    cout << "in" << sock << endl;

    WEndPointInfo en;
    int           res = Accept(sock, &en);

    if(res == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
        exit(-1);
    } else {
        cout << "Accept ok" << endl;
    }

    auto info = WEndPointInfo::Dump(en);
    cout << "client : ip " << std::get<0>(info) << " port:" << std::get<1>(info) << endl;

    auto t = (test_s *)data;
    t->f(t->n);
};

void test_wepoll() {
    cout << "test wepoll " << endl;
    auto ep = std::make_shared<WEpoll<test_s>>();
    ep->Init();
    ep->read_ = r_cb;

    auto sock = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    SetSocketReuseAddr(sock);
    SetSocketReusePort(sock);
    int res = Bind(sock, "0:0:0:0:0:0:0:0", 4000, 0);

    if(!res) {
        cout << "bind error : " << errno << endl;
        return;
    } else {
        cout << "bind ok" << endl;
    }

    res = listen(sock, 5);
    if(res == -1) {
        cout << "listen error : " << errno << endl;
        return;
    } else {
        cout << "listen ok" << endl;
    }

    test_s i{.f = [](int n) { cout << "heppy " << n << endl; }, .n = 3};
    auto   handler = std::make_unique<WEventHandle<test_s>::WEventHandler>();
    handler->socket_ = sock;
    handler->user_data_ = &i;
    handler->handle_ = ep;
    handler->SetEvents(HandlerEventType::EV_IN);
    handler->Enable();

    auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    res      = ConnectToHost(cli, "0:0:0:0:0:0:0:1", 4000, 0);

    if(!res) {
        cout << "[test_wepoll]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
    }

    std::thread t([&ep]() { ep->Loop(); });
    t.join();
}


/**
 * test_tcpchannel
 */
auto in_cb = [](socket_t sock, WBaseChannel *data) {
    auto *ch = (ReadChannel *)data;
    // cout << "get channel call channel in" << std::endl;
    ch->ChannelIn();
};
auto out_cb = [](socket_t sock, WBaseChannel *data) {
    auto *ch = (WriteChannel *)data;
    // cout << "get channel call channel in" << std::endl;
    ch->ChannelOut();
};

class TestSession : public WChannel::Listener {
public:
    TestSession(std::shared_ptr<WChannel> ch_) : ch(ch_) {}
    virtual void onChannelConnect(std::shared_ptr<WChannel>) {}
    virtual void onChannelDisConnect() {}
    virtual void onReceive(const uint8_t *message, uint64_t message_len) {
        // cout << "recv " << std::string((char *)message, (int)message_len) << " size " << message_len << endl;
        ch->Send(message, message_len);
        ch->Send(message, message_len);
        ch->Send(message, message_len);
        ch->Send(message, message_len);
        ch->Send(message, message_len);
        ch->Send(message, message_len);
        ch->Send(message, message_len);
        ch->Send(message, message_len);
    }
    virtual void onError(const char *err_message) { std::cout << err_message << endl; }

private:
    std::shared_ptr<WChannel> ch;
};

std::shared_ptr<TestSession> se;

auto ac_cb = [](const WEndPointInfo        &local,
                const WEndPointInfo        &remote,
                std::unique_ptr<ev_hdler_t> handler) {
    auto info = WEndPointInfo::Dump(remote);

    // cout << "recv : info " << std::get<0>(info) << " " << std::get<1>(info) << std::endl;
    auto ch = std::make_shared<WChannel>(local, remote, std::move(handler));
    ch->SetRecvBufferMaxSize(10, 1000);
    se = std::make_shared<TestSession>(ch);
    ch->SetListener(se);
};

void test_tcpchannel() {
    cout << "test channel " << endl;

    auto ep = std::make_shared<WEpoll<WBaseChannel>>();
    ep->Init();
    ep->read_  = in_cb;
    ep->write_ = out_cb;

    WEndPointInfo local_ed = *WEndPointInfo::MakeWEndPointInfo("0:0:0:0:0:0:0:0", 4000, AF_FAMILY::INET6);

    auto accp_channel      = new WAccepterChannel(ep);
    accp_channel->Start(local_ed);
    accp_channel->OnAccept = ac_cb;

    auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    bool res = ConnectToHost(cli, "0:0:0:0:0:0:0:1", 4000, 0);

    if(!res) {
        cout << "[test_tcpchannel]connect error : " << strerror(errno) << endl;
        return;
    } else {
        cout << "connect ok" << endl;
        ::send(cli, "123123", 6, 0);
    }

    std::thread thr1([cli]() {
        // ::send(cli, "hello", 5, 0);
        int  total = 0;
        char buf[1500];
        while(true) {
            auto l = ::recv(cli, buf, 1500, 0);
            total += l;
            // clang-format off
            cout 
                // << "cli recv :" << std::string(buf, l) 
                << " total : " << total 
                << endl;
            // clang-format on
        }
    });
    
    WTimer t(ep);
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
    t.Start(100, 1);

    std::thread thr([&ep]() { ep->Loop(); });
    thr.join();
}

/**
 * test_udp
 */
void test_udp() {
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

        len = RecvFrom(cli, (uint8_t *)cli_buf, 1500, &srv_);
        if(len <= 0) {
            std::cout << "cli recv from err " << ErrorToString(GetError()) << endl;
        } else {
            auto [ip, port] = WEndPointInfo::Dump(srv_);
            cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() << endl;
        }

        sendto(cli, send_msg, strlen(send_msg), 0, srv_ed.GetAddr(), srv_ed.GetSockSize());

        len = RecvFrom(cli, (uint8_t *)cli_buf, 1500, &srv_);
        if(len <= 0) {
            std::cout << "cli recv from err " << ErrorToString(GetError()) << endl;
        } else {
            auto [ip, port] = WEndPointInfo::Dump(srv_);
            cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() << endl;
        }
    });
    th1.join();
}


/**
 * test_udpchannel
 */

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

void test_udpchannel() {
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

    auto udp_srv = std::make_unique<WUDPChannel>(ep);
    udp_srv->Start(srv_ed, cli_ed);
    // auto udp_srv = new WUDPChannel(srv_ed, cli2_ed, &ep);

    std::shared_ptr<udpSession> sess = std::make_shared<udpSession>();
    sess->cli_ed = cli_ed;
    sess->srv_ed = srv_ed;
    udp_srv->SetListener(sess);
    sess->udp_chl = std::move(udp_srv);

    std::thread th1([&]() { ep->Loop(); });

    std::thread th2([&]() {
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


        len = RecvFrom(cli, (uint8_t *)cli_buf, 1500, &srv_);
        if(len <= 0) {
            std::cout << "cli recv from err " << ErrorToString(GetError()) << endl;
        } else {
            auto [ip, port] = WEndPointInfo::Dump(srv_);
            cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() << endl;
        }

        sendto(cli, send_msg, strlen(send_msg), 0, srv_ed.GetAddr(), srv_ed.GetSockSize());

        len = RecvFrom(cli, (uint8_t *)cli_buf, 1500, &srv_);
        if(len <= 0) {
            std::cout << "cli recv from err " << ErrorToString(GetError()) << endl;
        } else {
            auto [ip, port] = WEndPointInfo::Dump(srv_);
            cout << "cli recv [" << ip << " : " << port << "] " << std::string(cli_buf, len).c_str() << endl;
        }
    });
    th1.join();
}


/**
 * test_tcpserver
 */
// auto acc_cb = [](wlb::network::socket_t socket, wlb::network::WEndPointInfo &endpoint) -> bool {
//     // cout << "accpt : " << socket << " info " << endpoint.ip_address << " " << endpoint.port << std::endl;

//     // ch = new WChannel(socket, endpoint);
//     return true;
// };

void sin_handle(int signal) { exit(-1); }
// using namespace wlb::debug;

// void test_tcpserver() {
//     // debugger->Init(1000);

//     signal(SIGINT, sin_handle);

//     WSingleTcpServer ser;
//     ser.SetOnAccept(acc_cb);
//     ser.AddAccepter(*WEndPointInfo::MakeWEndPointInfo("0:0:0:0:0:0:0:0", 4000, AF_FAMILY::INET6));


//     auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
//     auto res = ConnectToHost(cli, "0:0:0:0:0:0:0:1", 4000, 0);

//     if(!res) {
//         cout << "[test_tcpserver]connect error : " << strerror(errno) << endl;
//     } else {
//         cout << "connect ok" << endl;
//         std::thread([&]() {
//             int len = ::send(cli, "123123", 6, 0);
//             cout << "send ok " << len << endl;
//             char arr[1024];
//             len = ::recv(cli, arr, 1024, 0);
//             cout << "recv arr : " << arr << std::endl;
//             cout << "recv ok " << len << endl;
//         }).detach();
//     }

//     auto t = ser.NewTimer();
//     // NEWADD;
//     // t->OnTime = []() { cout << "hello " << endl; };
//     // t->Start(1000, 1000);
//     delete t;
//     // DELADD;


//     ser.Start();
//     ser.Join();
// }

auto acc2_cb = [](wlb::network::socket_t socket, wlb::network::WEndPointInfo &endpoint) -> bool {
    // cout << "accpt : " << socket << " info " << endpoint.ip_address << " " << endpoint.port << std::endl;
    // TODO: 使用抽象工厂模式
    // ch = new MyChannel(socket, endpoint);
    return true;
};
void test_myChannel() {
    // debugger->Init(1000);

    // signal(SIGINT, sin_handle);

    // WSingleTcpServer ser;
    // ser.SetOnAccept(acc2_cb);

    // auto linfo = WEndPointInfo::MakeWEndPointInfo("0:0:0:0:0:0:0:0", 8000, wlb::network::AF_FAMILY::INET6);
    // if(linfo == nullptr) {
    //     cout << "[test_myChannel]MakeWEndPointInfo linfo error : " << strerror(errno) << endl;
    //     return;
    // }

    // bool ok = ser.AddAccepter(*linfo);
    // if(!ok) {
    //     cout << "[test_myChannel]AddAccepter error : " << strerror(errno) << endl;
    // }
    // ser.SetSessionFactory(new MySessionFactory);


    // auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);

    // auto cinfo = WEndPointInfo::MakeWEndPointInfo("0:0:0:0:0:0:0:0", 8000, wlb::network::AF_FAMILY::INET6);
    // if(cinfo == nullptr) {
    //     cout << "[test_myChannel]MakeWEndPointInfo cinfo error : " << strerror(errno) << endl;
    //     return;
    // }
    // auto res = ConnectToHost(cli, *cinfo);

    // if(!res) {
    //     cout << "[test_myChannel]connect error : " << strerror(errno) << endl;
    // } else {
    //     cout << "connect ok" << endl;
    //     std::thread([&]() {
    //         int len = ::send(cli, "asdasd", 6, 0);
    //         cout << "send ok " << len << endl;
    //         char arr[1024];
    //         len = ::recv(cli, arr, 1024, 0);
    //         if(len > 0) {
    //             cout << "recv arr : " << arr << std::endl;
    //             cout << "recv ok " << len << endl;
    //         } else {
    //             int err = errno;
    //             cout << "recv len == 0 error " << err << " " << strerror(err) << endl;
    //         }
    //         close(cli);
    //     }).detach();
    // }

    // auto t = ser.NewTimer();
    // // NEWADD;
    // // t->OnTime = []() { cout << "hello " << endl; };
    // // t->Start(1000, 1000);
    // delete t;
    // // DELADD;


    // ser.Start();
    // ser.Join();
}
