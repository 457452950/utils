#include <iostream>
#include <signal.h>

#include "AsyncLogger.h"

using namespace std;
using namespace wlb::Log;


void signal_handle(int sign) {
    cout << "singal " << sign << endl;
    Logger::Stop();
}

int main() {
    signal(SIGINT, signal_handle);

    char *h = "hello ";

    LOG(L_DEBUG) << "debug logger" << h << 123;
    Logger::Init(LOG_TYPE::L_FILE | LOG_TYPE::L_STDOUT, LOG_LEVEL::L_DEBUG, "log_demo");
    cout << "logger init over" << endl;
    LOG(L_DEBUG) << "debug logger" << h << 123;

    Logger::Wait2Exit();
    return 0;
}
