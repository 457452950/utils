#include <csignal>
#include <iostream>
#include <memory>

#include <sys/uio.h>

#include "test_timer.h"
#include "test_ipv6.h"

#include "test_tcp_connection.h"
#include "test_tcp_aconnection.h"
#include "test_tcpserver.h"
#include "test_udp.h"
#include "test_udpchannel.h"


#include "wutils/Debugger.hpp"
#include "wutils/network/NetWork.h"

using namespace std;
using namespace wutils::network;

int main() {

    Logger::GetInstance()->LogCout()->SetLogLevel(LDEBUG)->Start();

    // signal(SIGPIPE, handle_pipe); // 自定义处理函数

    test_timer();
    test_ipv6();

    test_connection();
    test_aconnection();
    // test_udp();
    // test_udpchannel();
    // test_tcpserver();
    // test_myChannel();
}
