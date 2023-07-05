#ifndef UTILS_DEMO_WNET_TEST_TIMER_H
#define UTILS_DEMO_WNET_TEST_TIMER_H

#include "wutils/logger/StreamLogger.h"
#include "wutils/network/IOEvent.h"

using namespace wutils::log;

int tag = 0;

inline void test_timer() {
    Logger::GetInstance()->LogCout()->SetLogLevel(LDEBUG)->Start();
    auto handle = std::make_shared<Epoll<IOEvent>>();
    setCommonCallBack(handle.get());

    handle->Init();

    auto timer = std::make_shared<Timer>(handle);

    timer->OnTime = [timer]() {
        using namespace std::chrono;
        LOG(LINFO, "timer") << "time out"
                            << duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        ++tag;
        if(tag == 2) {
            timer->Start(2000, 2000);
        }
    };
    timer->Start(1000, 1000);

    handle->Loop();
}


#endif // UTILS_DEMO_WNET_TEST_TIMER_H
