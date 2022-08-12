//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
#define UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_

#include "WNetWorkUtils.h"
#include "WEvent.h"
#include "WSelect.h"
#include "WEpoll.h"
#include "WChannel.h"
#include "WSingleTcpServer.h"

#include "../WOS.h"

namespace wlb::network {


enum class EventHanleType {
#ifdef OS_IS_LINUX
    SELECT,
    EPOLL,
#endif
};



/* WEvent.h */
template <typename UserData>
class WEventHandle;

WEventHandle<WBaseChannel> *CreateNetHandle();


/* WSelect.h */
template <typename UserData>
class WSelect;


/* WEpoll.h */
class WBaseEpoll;
template<typename UserData>
class WEpoll;


/* WChannel.h */
class WTimer;
class WAccepterChannel;
class WChannel;


/* WsingleTcpServer.h */
class WSingleTcpServer;



} // namespace wlb::network

#endif // UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
