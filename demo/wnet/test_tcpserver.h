#ifndef UTILS_DEMO_WNET_TEST_IOVEC_H
#define UTILS_DEMO_WNET_TEST_IOVEC_H

#include <iostream>

#include "wutils/network/NetWork.h"

using namespace std;
using namespace wutils::network;


/**
 * test_tcpserver
 */
// auto acc_cb = [](wutils::network::socket_t socket, wutils::network::EndPointInfo &endpoint) -> bool {
//     // cout << "accpt : " << socket << " info " << endpoint.ip_address << " " << endpoint.port << std::endl;

//     // ch = new Channel(socket, endpoint);
//     return true;
// };


// using namespace wutils::debug;

// void test_tcpserver() {
//     // debugger->Init(1000);

//     signal(SIGINT, sin_handle);

//     SingleTcpServer ser;
//     ser.SetOnAccept(acc_cb);
//     ser.AddAccepter(*EndPointInfo::MakeWEndPointInfo("0:0:0:0:0:0:0:0", 4000, AF_FAMILY::INET6));


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

#endif