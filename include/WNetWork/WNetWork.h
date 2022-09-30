//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
#define UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_

#include "WChannel.h"
#include "WEpoll.h"
#include "WEvent.h"
#include "WNetFactory.h"
#include "WNetWorkDef.h"
#include "WNetWorkUtils.h"
#include "WSelect.h"
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

template <typename UserData>
class WEpoll;


/* WChannel.h */
class WTimer;
class WAccepterChannel;
class WChannel;


/* WNetFactory.h */
class WChannelFactory;


/* WSingleTcpServer.h */
class WSingleTcpServer;


} // namespace wlb::network

#endif // UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
