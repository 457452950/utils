//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
#define UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_

#include "WNetWorkUtils.h"
#include "WNetWorkDef.h"
#include "WEvent.h"
#include "WSelect.h"
#include "WEpoll.h"
#include "WChannel.h"
#include "WNetFactory.h"
#include "WSingleTcpServer.h"

#include "../WOS.h"

namespace wlb::network {


/* WEvent.h */
template <typename UserData>
class WEventHandle;


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


/* WNetFactory.h */
class WNetFactory;


/* WsingleTcpServer.h */
class WSingleTcpServer;



} // namespace wlb::network

#endif // UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
