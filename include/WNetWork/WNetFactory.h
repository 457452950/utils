#pragma once
#ifndef UTILS_WNETFACTORY_H
#define UTILS_WNETFACTORY_H

#include "WChannel.h"
#include "WNetWorkDef.h"
#include "WSelect.h"
#include "WEpoll.h"

namespace wlb::network {

class WNetFactory final {
public:
    WEventHandle<WBaseChannel>* CreateNetHandle(HandleType type) {
        switch (type)
        {
        case HandleType::SELECT:
            return new WSelect<WBaseChannel>();
            
        case HandleType::EPOLL:
            return new WEpoll<WBaseChannel>();
        
        default:
            abort();
        }
        return nullptr;
    }

    /* 单例模式 */
public:
    static WNetFactory* GetInstance() {
        return instance_;
    }
    ~WNetFactory() {}     

private:
    static WNetFactory* instance_;
    WNetFactory() {}

};

inline WNetFactory* WNetFactory::instance_ = new WNetFactory();




}

#endif
