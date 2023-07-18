#ifndef UTILS_DEMO_WNET_TEST_TIMER_H
#define UTILS_DEMO_WNET_TEST_TIMER_H

#include "wutils/logger/StreamLogger.h"
#include "wutils/network/NetWork.h"

using namespace wutils::log;

int tag = 0;

inline void test_timer() {
    using namespace std::chrono;

    auto handle = std::make_shared<event::EpollContext>();

    handle->Init();

    auto timer = std::make_shared<Timer>(handle);

    timer->OnTime = [timer]() {
        using namespace std::chrono;
        LOG(LINFO, "timer") << "time out"
                            << duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        static int i = 0;
        if(!i++)
            assert(timer->Loop(100ms, 10));
    };
    LOG(LINFO, "timer") << "start" << duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    assert(timer->Once(1s));
    assert(timer->IsActive());

    handle->Loop();
}


#endif // UTILS_DEMO_WNET_TEST_TIMER_H
