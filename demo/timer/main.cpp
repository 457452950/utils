#include <iostream>
#include <csignal>

#include "wutils/logger/Logger.h"
#include "wutils/logger/StreamLogger.h"
#include "wutils/timer/EasyContext.h"

using namespace std;
using namespace wutils::log;
using namespace wutils;

int main(int argc, char **argv) {
    // clang-format off
    Logger::GetInstance()
        ->LogCout()
        ->SetLogLevel(LDEBUG)
        ->Start();
    // clang-format on

    auto wheel = timer::EasyContext::Create();

    LOG(LINFO, "main") << "start";
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 1min"; }, 1min);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 1min"; }, 1min);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 1min"; }, 1min);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 1s"; }, 1s);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 1s"; }, 1s);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 5s"; }, 5s);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 5s"; }, 5s);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 20ms"; }, 20ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 23ms"; }, 23ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 22ms"; }, 22ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    LOG(LINFO, "main") << "add";
    wheel->AddTask([]() { LOG(LINFO, "main") << "time out 200ms"; }, 200ms);
    LOG(LINFO, "main") << "add";

    while(wheel->TaskCounts()) {
        auto t = wheel->DoTasks();
        if(t.count() > 0)
            LOG(LINFO, "main") << "sleep " << t.count();
        if(t.count() == -1) {
            LOG(LINFO, "main") << "all is done ";
            break;
        }
        std::this_thread::sleep_for(t);
    }


    Logger::GetInstance()->StopAndWait();

    return 0;
}
