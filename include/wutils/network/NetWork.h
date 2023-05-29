//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_NETWORK_H
#define UTILS_NETWORK_H

#include "Epoll.h"
#include "Event.h"
#include "NetFactory.h"
#include "NetWorkDef.h"
#include "NetWorkUtils.h"
#include "Select.h"
#include "SingleTcpServer.h"

#include "wutils/OS.h"

namespace wutils::network {


/* WEvent.h */
template <typename UserData>
class EventHandle;


/* Select.h */
template <typename UserData>
class Select;


/* Epoll.h */
class BaseEpoll;

template <typename UserData>
class Epoll;


/* Channel.h */
class Timer;
class AcceptorChannel;
class UDPPointer;
class UDPChannel;
class Channel;


/* NetFactory.h */
template <typename Channel>
struct TCPEvFactory;


/* SingleTcpServer.h */
class SingleTcpServer;


} // namespace wutils::network

#endif // UTILS_NETWORK_H
