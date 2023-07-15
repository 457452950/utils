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
#include "Tools.h"
#include "wutils/network/base/Native.h"
#include "wutils/network/io_event/IOContext.h"

namespace wutils::network {


/* IOContext.h */
template <typename UserData>
class IOContext;


/* Select.h */
template <typename UserData>
class Select;


/* Epoll.h */
class Epoll;

template <typename UserData>
class Epoll;


/* IOEvent.h */
class Timer;
class UDPPointer;
class UDPChannel;
class Channel;

/* Acceptor.h */
namespace tcp {
class Acceptor;
} // namespace tcp

} // namespace wutils::network

#endif // UTILS_NETWORK_H
