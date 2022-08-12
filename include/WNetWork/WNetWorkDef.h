#pragma once 
#ifndef WNETWORKDEF_H
#define WNETWORKDEF_H

#include "../WOS.h"

namespace wlb::network {

#ifdef OS_IS_LINUX
  #define SERVER_USE_EPOLL
#else
  #define SERVER_USE_SELECT
#endif

} // namespace wlb::network



#endif



