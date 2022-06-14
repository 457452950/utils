//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
#define UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_

#include "WEpoll.h"
#include "WEventHandle.h"
#include "WNetWorkUtils.h"

namespace wlb::network {


enum class EventHanleType {
#ifdef OS_IS_LINUX
    SELECT,
    EPOLL,
#endif
};

inline WEventHandle *MakeEventHandle(EventHanleType type);


} // namespace wlb::network

#endif // UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
