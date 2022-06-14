#include <csignal>
#include <iostream>

#include "WNetWork/WNetWork.h"

using namespace std;
using namespace wlb::network;

void handle_pipe(int singn) { cout << "singn" << endl; }

void test_ipv6();
void test_wepoll();

int main() {

    signal(SIGPIPE, handle_pipe); // 自定义处理函数

    // test_ipv6();
    test_wepoll();
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
        cout << "connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
    }

    WEndPointInfo en;
    res = Accept(sock, &en, false);

    if(res == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
    } else {
        cout << "Accept ok" << endl;
    }

    cout << "client : ip " << en.ip_address << " port:" << en.port << endl;
}

struct test_s {
    std::function<void(int)> f;
    int                      n;
};


WEpoll::read_callback r_cb = [](base_socket_type sock, WEpoll::user_data_ptr data) {
    cout << "in" << sock << endl;

    WEndPointInfo en;
    int           res = Accept(sock, &en, false);

    if(res == -1) {
        cout << "Accept error : " << strerror(errno) << endl;
        exit(-1);
    } else {
        cout << "Accept ok" << endl;
    }

    cout << "client : ip " << en.ip_address << " port:" << en.port << endl;

    auto t = (test_s*)data;
    t->f(t->n);
};

void test_wepoll() {
    cout << "test wepoll " << endl;
    WEpoll ep;
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

    test_s i {
        .f = [](int n) { cout << "heppy " << n << endl; },
        .n = 3
    };
    auto list = ep.NewSocket(sock, EventType::EV_READ | EventType::EV_Error, &i);

    auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    res      = ConnectToHost(cli, "0:0:0:0:0:0:0:1", 4000, 0);

    if(!res) {
        cout << "connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
    }

    ep.Start();
    ep.Join();
}
