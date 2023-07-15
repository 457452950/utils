//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_NETWORK_H
#define UTILS_NETWORK_H

#include "Factory.h"
#include "base/Definition.h"
#include "base/Native.h"
#include "io_event/IOContext.h"
#include "wutils/network/easy/Acceptor.h"
#include "wutils/network/easy/Epoll.h"
#include "wutils/network/easy/Select.h"
#include "wutils/network/easy/Tcp.h"
#include "wutils/network/easy/Tools.h"
#include "wutils/network/easy/Udp.h"

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

/* easy/Epoll.h */
class Epoll;

namespace event {
/* io_context/IOContext.h */
class IOContext;
class IOEvent;
/* io_context/SelectContext.h */
class SelectContext;
} // namespace event


} // namespace wutils::network

#endif // UTILS_NETWORK_H
