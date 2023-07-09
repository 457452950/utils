//
// Created by wang on 22-5-5.
//
#pragma once
#ifndef UTILS_NETWORK_H
#define UTILS_NETWORK_H


#include <cstdint>

#include "wutils/OS.h"

namespace wutils::network {

#ifdef OS_IS_LINUX
#define SERVER_USE_EPOLL
#else
#define SERVER_USE_SELECT
#endif


enum class HandleType : uint8_t {
    SELECT,
    EPOLL,
};

#ifdef SERVER_USE_EPOLL
const auto    default_handle_type = HandleType::EPOLL;
#elif defined SERVER_USE_SELECT
const auto default_handle_type = HandleType::SELECT;
#endif


#define MAX_CHANNEL_SEND_SIZE (16 * 1024ULL)         // 16k
#define MAX_CHANNEL_RECV_BUFFER_SIZE (320 * 1024ULL) // 320k
#define MAX_CHANNEL_SEND_BUFFER_SIZE (160 * 1024ULL) // 160k

#define MAX_LAN_UDP_PACKAGE_LEN 1472
#define MAX_WAN_UDP_PACKAGE_LEN 548
#define MAX_UDP_BUFFER_LEN 1500


} // namespace wutils::network


#include "Factory.h"
#include "IO_Context.h"
#include "SingleTcpServer.h"
#include "wutils/network/base/Defined.h"
#include "wutils/network/base/Epoll.h"
#include "wutils/network/base/Select.h"

namespace wutils::network {


/* Event.h */
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
