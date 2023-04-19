#include <csignal>
#include <iostream>

#include "Channel.h"
#include "WDebugger.hpp"
#include "WNetWork/WNetWork.h"

using namespace std;
using namespace wlb::network;

void handle_pipe(int signal) { cout << "signal" << endl; }

void test_ipv6();
void test_wepoll();
void test_channel();
void test_tcpserver();
void test_myChannel();

int main() {

    signal(SIGPIPE, handle_pipe); // 自定义处理函数

    // test_ipv6();
    // test_wepoll();

    // test_channel();
    // test_tcpserver();
    test_myChannel();
}


void test_ipv6() {

    auto sock = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    int  res  = Bind(sock, "0:0:0:0:0:0:0:0", 4000, 0);

    if(!res) {
        cout << "bind error : " << errno << endl;
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

struct test_s {
    std::function<void(int)> f;
    int                      n;
};

auto r_cb = [](base_socket_type sock, WSelect<test_s>::user_data_ptr data) {
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
    WSelect<test_s> ep;
    ep.read_ = r_cb;

    auto sock = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    SetSocketReuseAddr(sock);
    SetSocketReusePort(sock);
    int res = Bind(sock, "0:0:0:0:0:0:0:0", 4000, 0);

    if(!res) {
        cout << "bind error : " << errno << endl;
    } else {
        cout << "bind ok" << endl;
    }

    res = listen(sock, 5);
    if(res == -1) {
        cout << "listen error : " << errno << endl;
    } else {
        cout << "listen ok" << endl;
    }

    test_s i{.f = [](int n) { cout << "heppy " << n << endl; }, .n = 3};
    auto   list = ep.NewSocket(new typename WEventHandle<test_s>::option_type{
              .socket_ = sock, .user_data_ = &i, .events_ = KernelEventType::EV_IN});

    auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    res      = ConnectToHost(cli, "0:0:0:0:0:0:0:1", 4000, 0);

    if(!res) {
        cout << "[test_wepoll]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
    }

    ep.Start();
    ep.Join();
}


auto in_cb = [](base_socket_type sock, WBaseChannel *data) {
    auto *ch = (ReadChannel *)data;
    // cout << "get channel call channel in" << std::endl;
    ch->ChannelIn();
};

event_context_t       con;
WEpoll<WBaseChannel>  ep;
WSelect<WBaseChannel> sl;
WChannel             *ch;

auto ac_cb = [](wlb::network::base_socket_type socket, wlb::network::WEndPointInfo &endpoint) -> bool {
    auto info = WEndPointInfo::Dump(endpoint);

    cout << "recv : " << socket << " info " << std::get<0>(info) << " " << std::get<1>(info) << std::endl;
    // ch = new WChannel(socket, endpoint);
    return true;
};


void test_channel() {
    cout << "test channel " << endl;

    ep.read_ = in_cb;
    con      = {
                 .onAccept      = ac_cb,
                 .onAcceptError = [](int err) { cout << "error :" << strerror(err) << endl; },
                 .event_handle_ = &ep,
    };


    auto sock = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    SetSocketReuseAddr(sock);
    SetSocketReusePort(sock);
    WEndPointInfo local_ed = *WEndPointInfo::MakeWEndPointInfo("0:0:0:0:0:0:0:0", 4000, AF_FAMILY::INET6);
    int           res      = Bind(sock, local_ed);

    if(!res) {
        cout << "bind error : " << errno << endl;
    } else {
        cout << "bind ok" << endl;
    }

    res = listen(sock, 5);
    if(res == -1) {
        cout << "listen error : " << errno << endl;
    } else {
        cout << "listen ok" << endl;
    }

    auto accp = WAccepterChannel(sock, local_ed, &con);

    auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    res      = ConnectToHost(cli, "0:0:0:0:0:0:0:1", 4000, 0);

    if(!res) {
        cout << "[test_channel]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
        ::send(cli, "123123", 6, 0);
    }

    WTimer t(&ep);
    t.OnTime = []() { cout << "ontime!!!" << endl; };
    t.Start(1000, 1000);


    ep.Start();
    ep.Join();
}

auto acc_cb = [](wlb::network::base_socket_type socket, wlb::network::WEndPointInfo &endpoint) -> bool {
    // cout << "accpt : " << socket << " info " << endpoint.ip_address << " " << endpoint.port << std::endl;

    // ch = new WChannel(socket, endpoint);
    return true;
};

void sin_handle(int signal) { exit(-1); }
using namespace wlb::debug;

void test_tcpserver() {
    // debugger->Init(1000);

    signal(SIGINT, sin_handle);

    WSingleTcpServer ser;
    ser.SetOnAccept(acc_cb);
    ser.AddAccepter(*WEndPointInfo::MakeWEndPointInfo("0:0:0:0:0:0:0:0", 4000, AF_FAMILY::INET6));


    auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    auto res = ConnectToHost(cli, "0:0:0:0:0:0:0:1", 4000, 0);

    if(!res) {
        cout << "[test_tcpserver]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
        std::thread([&]() {
            int len = ::send(cli, "123123", 6, 0);
            cout << "send ok " << len << endl;
            char arr[1024];
            len = ::recv(cli, arr, 1024, 0);
            cout << "recv arr : " << arr << std::endl;
            cout << "recv ok " << len << endl;
        }).detach();
    }

    auto t = ser.NewTimer();
    // NEWADD;
    // t->OnTime = []() { cout << "hello " << endl; };
    // t->Start(1000, 1000);
    delete t;
    // DELADD;


    ser.Start();
    ser.Join();
}

auto acc2_cb = [](wlb::network::base_socket_type socket, wlb::network::WEndPointInfo &endpoint) -> bool {
    // cout << "accpt : " << socket << " info " << endpoint.ip_address << " " << endpoint.port << std::endl;
    // TODO: 使用抽象工厂模式
    // ch = new MyChannel(socket, endpoint);
    return true;
};
void test_myChannel() {
    // debugger->Init(1000);

    signal(SIGINT, sin_handle);

    WSingleTcpServer ser;
    ser.SetOnAccept(acc2_cb);
    bool ok = ser.AddAccepter(*WEndPointInfo::MakeWEndPointInfo("0:0:0:0:0:0:0:0", 4000, wlb::network::AF_FAMILY::INET6));
    if (!ok) {
        cout << "[test_myChannel]AddAccepter error : " << strerror(errno) << endl;
    }
    ser.SetSessionFactory(new MySessionFactory);


    auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    auto res = ConnectToHost(cli, "0:0:0:0:0:0:0:1", 4000, false);

    if(!res) {
        cout << "[test_myChannel]connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
        std::thread([&]() {
            int len = ::send(cli, "asdasd", 6, 0);
            cout << "send ok " << len << endl;
            char arr[1024];
            len = ::recv(cli, arr, 1024, 0);
            if(len > 0) {
                cout << "recv arr : " << arr << std::endl;
                cout << "recv ok " << len << endl;
            } else {
                int err = errno;
                cout << "recv len == 0 error " << err << " " << strerror(err) << endl;
            }
            close(cli);
        }).detach();
    }

    auto t = ser.NewTimer();
    // NEWADD;
    // t->OnTime = []() { cout << "hello " << endl; };
    // t->Start(1000, 1000);
    delete t;
    // DELADD;


    ser.Start();
    ser.Join();
}
