#include "WDebugger.hpp"
#include <iostream>

namespace wlb::debug {

// WTimerHandler                 *debuggerHandler = new wlb::network::WTimerEpoll;
Debugger                      *debugger        = new Debugger();
[[maybe_unused]] WTimeDebugger time_debug;

bool Debugger::Init(long timeout) {
    if(!debuggerHandler->Init()) {
        return false;
    }

    this->_timer = new(std::nothrow) WTimer(this, debuggerHandler);
    if(this->_timer == nullptr) {
        return false;
    }

    if(!this->_timer->Start(timeout, timeout)) {
        return false;
    }

    this->_isActive = true;

    this->_thread = new(std::nothrow) std::thread(&Debugger::Loop, this);
    if(this->_thread == nullptr) {
        return false;
    }

    return true;
}

void Debugger::Destroy() {
    this->_isActive = false;

    if(this->_timer != nullptr) {
        delete this->_timer;
        this->_timer = nullptr;
    }

    if(this->_thread != nullptr) {
        delete this->_thread;
        this->_thread = nullptr;
    }

    this->_memberMap.clear();
}

void Debugger::MemberAdd(const std::string &name) { this->_memberMap[name]++; }

void Debugger::MemberRemove(const std::string &name) { this->_memberMap[name]--; }

void Debugger::OnTime(WTimer *timer) {
    for(auto &it : this->_memberMap) {
        std::cout << it.first << " " << it.second << std::endl;
    }
}

void Debugger::Loop() const {
    while(this->_isActive) {
        debuggerHandler->GetAndEmitTimer(-1);
    }
}

} // namespace wlb::debug
