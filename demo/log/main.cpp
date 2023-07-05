#include <iostream>
#include <signal.h>

#include "wutils/logger/Logger.h"

#define FORMAT

#ifdef STREAM
#include "wutils/logger/stream.h"
#elif defined FORMAT
#include "wutils/logger/FormatLogger.h"
#endif

using namespace std;
using namespace wutils::log;


void signal_handle(int sign) {
    cout << "signal " << sign << endl;
    Logger::GetInstance()->StopAndWait();
}

int main(int argc, char **argv) {
    signal(SIGINT, signal_handle);

    char *h = "hello ";
#ifdef STREAM
    LOG(LDEBUG, "main") << "debug logger" << h << 123;
#elif defined FORMAT
    LOG_DBG("main", "debug logger %s %d", h, 123);
#endif

    // clang-format off
    Logger::GetInstance()
        ->LogCout()
        ->LogFile("a/b/bcc/temp.abc.logg")
        ->SetLogLevel(LDEBUG)
        ->Start();
    // clang-format on

    cout << "logger init over" << endl;

    thread t([]() {
        char *h = "hello ";
        while(true) {
#ifdef STREAM
            LOG(LDEBUG, "main") << "debug logger " << h << 123 << 0.001 << 1e10;
            LOG(LINFO, "main") << "debug logger " << h << 123 << 0.001 << 1e10;
            LOG(LWARN, "main") << "debug logger " << h << 123 << 0.001 << 1e10;
            LOG(LERROR, "main") << "debug logger " << h << 123 << 0.001 << 1e10;
            LOG(LFATAL, "main") << "debug logger " << h << 123 << 0.001 << 1e10;
#elif defined FORMAT
            LOG_DBG("main", "debug logger %s %d %f %d", h, 123, 0.003, 1e11);
            LOG_WRN("main", "debug logger %s %d %f %d", h, 123, 0.003, 1e11);
            LOG_ERR("main", "debug logger %s %d %f %d", h, 123, 0.003, 1e11);
            LOG_FAL("main", "debug logger %s %d %f %d", h, 123, 0.003, 1e11);
            LOG_INF("main", "debug logger %s %d %f %d", h, 123, 0.003, 1e11);
#endif
            //            std::this_thread::sleep_for(10ms);
        }
        //        usleep(10);
    });


    t.join();
    Logger::GetInstance()->StopAndWait();

    return 0;
}
