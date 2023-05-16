//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_WNETWORK_H
#define UTILS_WNETWORK_H

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
template <typename Channel>
class WTCPEvFactory;


/* WSingleTcpServer.h */
class WSingleTcpServer;


} // namespace wlb::network

#endif // UTILS_WNETWORK_H
