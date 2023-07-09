#include <csignal>
#include <iostream>
#include <memory>

// #include "test_ipv6.h"
// #include "test_wepoll.h"
// #include "test_wselect.h"
//
// #include "test_aschannel.h"
// #include "test_mychannel.h"
// #include "test_tcpchannel.h"
// #include "test_tcpserver.h"
// #include "test_udp.h"
// #include "test_udpchannel.h"
//
// #include "test_timer.h"

// #include "Channel.h"
//  #include "wutils/Debugger.hpp"
//   #include "wutils/network/NetWork.h"

#include "wutils/network/Acceptor.h"
#include "wutils/network/Ip.h"
#include "wutils/network/UDP.h"
#include "wutils/network/base/EndPoint.h"

using namespace std;
using namespace wutils::network;

void test();

int main() {


    // signal(SIGPIPE, handle_pipe); // 自定义处理函数

    // test_preadv_pwritev();
    // test_iovec();

    // test_ipv6();
    //    test_wepoll();
    //    test_wselect();

    //    test_timer();

    //    test_tcpchannel();
    //    test_aschannel();
    // test_udp();
    // test_udpchannel();
    // test_tcpserver();
    // test_myChannel();

    test();
}

void test() {
    auto io_context = wutils::network::NetFactory::DefaultContext();

    auto acceptor = shared_ptr<event::Acceptor>(new event::Acceptor(io_context));
}
