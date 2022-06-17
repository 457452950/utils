//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
#define UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_

#include "WChannel.h"
#include "WEpoll.h"
#include "WEvent.h"
#include "WNetWorkUtils.h"
#include "WSelect.h"
#include "WSingleTcpServer.h"

#include "../WOS.h"

namespace wlb::network {


enum class EventHanleType {
#ifdef OS_IS_LINUX
    SELECT,
    EPOLL,
#endif
};

// enum class HandleType {
// #ifdef OS_IS_LINUX
//     SELECT = 1 << 0,
//     EPOLL  = 1 << 1,
// #endif
// };
class WSelect;
class WEpoll;

extern WEventHandle *CreateNetHandle(HandleType type);


// WChannel.h
class WTimer;
class WAccepterChannel;
class WChannel;

} // namespace wlb::network

#endif // UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
