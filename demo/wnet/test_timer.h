#ifndef UTILS_DEMO_WNET_TEST_TIMER_H
#define UTILS_DEMO_WNET_TEST_TIMER_H

#include <signal.h>

#include "wutils/logger/StreamLogger.h"
#include "wutils/network/Network.h"

using namespace wutils::log;
using namespace wutils::network;

namespace test_timer_config {

std::shared_ptr<event::IOContext> context;

void handle_pipe(int signal) {
    LOG(LINFO, "timer") << "signal";
    context->Stop();
}

} // namespace test_timer_config

inline void test_timer() {
    std::cout << "------------------ test timer ------------------------" << std::endl;

    using namespace test_timer_config;
    using namespace std::chrono;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
    signal(SIGINT, handle_pipe);  // 自定义处理函数

    context = event::EpollContext::Create();

    assert(context->Init());

    auto                 timer = Timer::Create(context);
    std::weak_ptr<Timer> t     = timer; // !!!

    timer->OnTime = [t]() {
        using namespace std::chrono;
        LOG(LINFO, "timer") << "time out"
                            << duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

        static int i = 0;
        if(!i++) {
            if(!t.expired())
                assert(t.lock()->Loop(100ms, 10).value() == 0);
        }
        if(i == 10) {
            LOG(LINFO, "timer") << "stop";
            context->Stop();
        }
    };
    LOG(LINFO, "timer") << "start" << duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    assert(timer->Once(1s).value() == 0);
    assert(timer->IsActive());

    context->Loop();
    //    timer->OnTime = []() {}; // 取消绑定中的智能指针, 或者使用弱指针
    context.reset();

    std::cout << "------------------ test timer end ------------------------" << std::endl;
}

#endif // UTILS_DEMO_WNET_TEST_TIMER_H
