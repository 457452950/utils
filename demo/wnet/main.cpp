#include <csignal>
#include <iostream>
#include <memory>

#include <sys/uio.h>

#include "test_preadv_pwritev.h"
#include "test_iovec.h"

#include "test_ipv6.h"
#include "test_wepoll.h"
#include "test_wselect.h"

#include "test_tcpchannel.h"
#include "test_udp.h"
#include "test_udpchannel.h"
#include "test_tcpserver.h"
#include "test_mychannel.h"

#include "Channel.h"
#include "WDebugger.hpp"
#include "WNetWork/WNetWork.h"
#include "WNetWork/stdIOVec.h"

using namespace std;
using namespace wlb::network;

int main() {


    // signal(SIGPIPE, handle_pipe); // 自定义处理函数

    // test_preadv_pwritev();
    // test_iovec();

    // test_ipv6();
    // test_wepoll();
    test_wselect();

    // test_tcpchannel();
    // test_udp();
    // test_udpchannel();
    // test_tcpserver();
    // test_myChannel();
}



