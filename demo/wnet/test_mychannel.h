#ifndef UTILS_DEMO_WNET_TEST_IOVEC_H
#define UTILS_DEMO_WNET_TEST_IOVEC_H

#include <iostream>

#include "WNetWork/WNetWork.h"

using namespace std;
using namespace wlb::network;



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


#endif