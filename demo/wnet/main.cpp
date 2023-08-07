#include <wutils/logger/StreamLogger.h>

#include "test_timer.h"
#include "test_ipv6.h"

#include "test_tcp_connection.h"
#include "test_tcp_aconnection.h"

#include "test_udp_point.h"
#include "test_async_udp_point.h"

#include "test_echo_tcp.h"


int main() {
    Logger::GetInstance()->LogCout()->SetLogLevel(LDEBUG)->Start();

    test_timer();
    test_ipv6();

    test_tcp_connection();
    test_aconnection();
    test_udp();
    test_async_udp();

    test_tcp_echo();

    Logger::GetInstance()->StopAndWait();
    return 0;
}
