//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_NETWORK_H
#define UTILS_NETWORK_H

#include "Acceptor.h"
#include "Epoll.h"
#include "Factory.h"
#include "Select.h"
#include "Tcp.h"
#include "Tools.h"
#include "Udp.h"
#include "base/Definition.h"
#include "base/Native.h"
#include "io_event/IOContext.h"

namespace wutils::network {

namespace tcp {
/* Acceptor.h */
class Acceptor;
/* Tcp.h */
class Socket;
} // namespace tcp

namespace udp {
/* Udp.h */
class Socket;
} // namespace udp

namespace event {
/* io_context/IOContext.h */
class IOContext;
class IOEvent;
/* io_context/SelectContext.h */
class SelectContext;
} // namespace event

/* Epoll.h */
class Epoll;


} // namespace wutils::network

#endif // UTILS_NETWORK_H
