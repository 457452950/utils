#include <csignal>
#include <iostream>

#include "WNetWork/WNetWork.h"

using namespace std;
using namespace wlb::network;

void handle_pipe(int singn) { cout << "singn" << endl; }

void test_ipv6();
void test_wepoll();
void test_channel();
void test_tcpserver();

int main() {

    signal(SIGPIPE, handle_pipe); // 自定义处理函数

    // test_ipv6();
    // test_wepoll();

    // test_channel();
    test_tcpserver();
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

auto r_cb = [](base_socket_type sock, WEpoll::user_data_ptr data) {
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

    auto t = (test_s *)data;
    t->f(t->n);
};


void test_wepoll() {
    cout << "test wepoll " << endl;
    WSelect ep;
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
    auto   list = ep.NewSocket(sock, KernelEventType::EV_IN, &i);

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


auto in_cb = [](base_socket_type sock, WEpoll::user_data_ptr data) {
    auto *ch = (ReadChannel *)data;
    cout << "get channel call channel in" << std::endl;
    ch->ChannelIn();
};

event_context_t con;
WEpoll          ep;
WSelect         sl;
WChannel       *ch;

auto ac_cb = [](wlb::network::base_socket_type socket, wlb::network::WEndPointInfo &endpoint) -> bool {
    cout << "recv : " << socket << " info " << endpoint.ip_address << " " << endpoint.port << std::endl;
    // ch = new WChannel(socket, endpoint, &con);
};


void test_channel() {
    cout << "test channel " << endl;

    ep.read_ = in_cb;
    con      = {
                 .onAccept      = ac_cb,
                 .onAcceptError = [](int err) { cout << "error :" << strerror(err) << endl; },
                 .onRead =
                         [](WChannel *channel, void *read_data, int64_t read_size) {
                        cout << "read len : " << read_size << "\n" << (char *)read_data << endl;
                    },
                 .max_read_size_ = 10240,
                 .event_handle_  = &ep,
    };


    auto sock = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    SetSocketReuseAddr(sock);
    SetSocketReusePort(sock);
    WEndPointInfo local_ed = {"0:0:0:0:0:0:0:0", 4000, 0};
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
        cout << "connect error : " << strerror(errno) << endl;
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
    cout << "accpt : " << socket << " info " << endpoint.ip_address << " " << endpoint.port << std::endl;
    return true;
    // ch = new WChannel(socket, endpoint, &con);
};

void test_tcpserver() {
    WSingleTcpServer ser;
    ser.SetOnAccept(acc_cb);
    ser.SetOnMessage([](WChannel *channel, void *read_data, int64_t read_size) {
        if(read_size) {
            cout << "read len : " << read_size << "\n"; //  << (char *)read_data << endl;
            channel->Send(read_data, read_size);
        } else {
            cout << "recv 0 "
                 << "del " << channel << std::endl;
            delete channel;
        }
    });
    ser.AddAccepter({"0:0:0:0:0:0:0:0", 4000, 0});


    auto cli = MakeSocket(AF_FAMILY::INET6, AF_PROTOL::TCP);
    auto res = ConnectToHost(cli, "0:0:0:0:0:0:0:1", 4000, 0);

    if(!res) {
        cout << "connect error : " << strerror(errno) << endl;
    } else {
        cout << "connect ok" << endl;
        new std::thread([&]() {
            ::send(cli, "123123", 6, 0);
            char arr[1024];
            ::recv(cli, arr, 1024, 0);
            cout << "recv arr : " << arr << std::endl;
        });
    }

    WTimer t(&ep);
    t.OnTime = []() { cout << "ontime!!!" << endl; };
    t.Start(1000, 1000);

    ser.Start();
    ser.Join();
}