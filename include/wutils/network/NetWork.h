//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_NETWORK_H
#define UTILS_NETWORK_H

#include "Factory.h"
#include "base/Definition.h"
#include "base/Native.h"
#include "easy/Acceptor.h"
#include "easy/Epoll.h"
#include "easy/Select.h"
#include "easy/Tcp.h"
#include "easy/Timer.h"
#include "easy/Tools.h"
#include "easy/Udp.h"
#include "io_event/EpollContext.h"
#include "io_event/IOContext.h"
#include "io_event/SelectContext.h"

#include "Acceptor.h"
#include "Timer.h"

namespace wutils::network {

namespace tcp {
/* easy/Acceptor.h */
class Acceptor;

/* easy/Tcp.h */
class Socket;
} // namespace tcp

namespace udp {
/* easy/Udp.h */
class Socket;
} // namespace udp

namespace timer {
/* easy/Timer.h */
class Socket;
} // namespace timer

/* easy/EpollContext.h */
class Epoll;

namespace event {
/* io_context/IOContext.h */
class IOContext;

/* io_context/IOEvent.h */
class IOEvent;

/* io_context/SelectContext.h */
class SelectContext;

/* io_context/SelectContext.h */
class EpollContext;
} // namespace event

/* Timer.h */
class Timer;

/* Acceptor.h */
class Acceptor;

} // namespace wutils::network

#endif // UTILS_NETWORK_H
