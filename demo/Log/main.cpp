#include <iostream>
#include <signal.h>

#include "AsyncLogger.h"

using namespace std;
using namespace wlb::Log;


void signal_handle(int sign) {
    cout << "singal " << sign << endl;
    Logger::Stop();
}

int main(int argc, char **argv) {
    signal(SIGINT, signal_handle);


    char *h = "hello ";
    LOG(LDEBUG, "main") << "debug logger" << h << 123;

    Logger::Init(LOG_TYPE::L_FILE, LOG_LEVEL::LDEBUG, "temp");
    // Logger::Init(LOG_TYPE::L_STDOUT, LOG_LEVEL::LDEBUG, argv[0]);
    cout << "logger init over" << endl;

    thread t([]() {
        char *h = "hello ";
        while(true) {
            LOG(LDEBUG, "main") << "debug logger " << h << 123;
        }
        usleep(10);
    });


    Logger::Wait2Exit();
    t.detach();

    return 0;
}
