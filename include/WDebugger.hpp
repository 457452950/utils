#pragma once

#include <atomic>
#include <unordered_map>
#include <thread>
#include <iostream>
#include "WOS.h"
#include "WTimer.hpp"
#include "WNetWork/WEpoll.hpp"

namespace wlb::debug
{

extern WTimerHandler* debuggerHandler;

class Debugger : public WTimerHandler::Listener
{
public:
    explicit Debugger() {};
    ~Debugger() {this->Destroy();};

    // class lifetime
    // init
    bool Init(long timeout); 
    inline bool IsActive() const {return this->_isActive;}
    void Destroy();

    void MemberAdd(const std::string& name);
    void MemberRemove(const std::string& name);

private:
    // override methods OnTime from WTimerHandler::Listener
    void OnTime(timerfd id) override;
    void Loop();

private:
    WTimer* _timer{nullptr};
    bool _isActive{false};
    std::thread* _thread{nullptr};

    std::unordered_map<std::string, std::atomic<int64_t>> _memberMap;
};

extern Debugger* debugger;

#define DEBUGADD(name) \
    if (debugger != nullptr && debugger->IsActive()) \
        debugger->MemberAdd(name);
        
#define DEBUGRM(name) \
    if (debugger != nullptr && debugger->IsActive()) \
        debugger->MemberRemove(name);

#define NEWADD \
    DEBUGADD("new")
#define DELADD \
    DEBUGADD("delete")\
    DEBUGRM("new")

#define NEWARRAYADD \
    DEBUGADD("new[]")
#define DELARRAYADD \
    DEBUGADD("delete[]")\
    DEBUGRM("new[]")


class WTimeDebugger
{
public:
    WTimeDebugger(){ this->_stamp = this->GetTime(); }

    void tick() { 
        int64_t now = this->GetTime();
        std::cout << now - this->_stamp << std::endl;
        this->_stamp = now;
    }

private:
    int64_t GetTime(){
        std::chrono::microseconds ms = std::chrono::duration_cast< std::chrono::microseconds >(
            std::chrono::system_clock::now().time_since_epoch());
        return ms.count();
    };

private:
    int64_t _stamp;
};

extern WTimeDebugger timedebug;

}   // namespace wlb::debug


