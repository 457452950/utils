#pragma once
#ifndef UTILS_NETWORK_DEF_H
#define UTILS_NETWORK_DEF_H


#include <cstdint>
#include <netinet/in.h>

#include "wutils/OS.h"

namespace wutils::network {

using socket_t = int;

namespace ip {

enum AF_FAMILY { INET = AF_INET, INET6 = AF_INET6 };
enum AF_PROTOL { TCP = IPPROTO_TCP, UDP = IPPROTO_UDP };


} // namespace ip

namespace timer {
using timer_t = int;
}


} // namespace wutils::network


#endif // UTILS_NETWORK_DEF_H
