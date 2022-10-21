#include <iostream>
#include <signal.h>

#include "AsyncLogger.h"

using namespace std;
using namespace wlb::Log;


void signal_handle(int sign) {
    cout << "singal " << sign << endl;
    Logger::Stop();
}

int main(int argc, char** argv) {
    signal(SIGINT, signal_handle);

    char *h = "hello ";

    LOG(L_DEBUG, "main") << "debug logger" << h << 123;
//    Logger::Init(LOG_TYPE::L_FILE | LOG_TYPE::L_STDOUT, LOG_LEVEL::L_DEBUG, argv[0]);
    Logger::Init(LOG_TYPE::L_STDOUT, LOG_LEVEL::L_DEBUG, argv[0]);
    cout << "logger init over" << endl;
    LOG(L_DEBUG, "main") << "debug logger " << h << 123;

    Logger::Wait2Exit();
    return 0;
}
