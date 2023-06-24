#include <csignal>
#include <iostream>
#include <memory>

// #include <sys/uio.h>

// #include "test_iovec.h"
// #include "test_preadv_pwritev.h"
//
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
//   #include "wutils/network/stdIOVec.h"

#include "wutils/network/Ip.h"
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
    ip::Udp::Socket<ip::v4> socket1;
    ip::Tcp::Socket<ip::v6> socket2;


    ip::v4::in_addr a;
    ip::IpStrToAddr<ip::V4>("127.0.0.1", &a);
    ip::IpStrToAddr("127.0.0.1", &a);

    std::string ip_str;
    ip::IpAddrToStr<ip::V4>(a, ip_str);
    ip::IpAddrToStr(a, ip_str);
    cout << "ip " << ip_str << endl;

    ip::v4::Address address1("127.0.0.1");
    ip::v6::Address address2;

    ip::v4::EndPointInfo e1;
    e1.Assign(address1, 4000);
    e1.Assign(ip::v4::Address{"127.0.0.1"}, 4000);
    e1.AsSockAddr();
}
