//
// Created by wang on 22-5-5.
//

#ifndef UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
#define UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_

#include "WNetWorkUtils.h"
#include "WMultiTcpServer.hpp"
#include "WBaseSession.hpp"

namespace wlb::NetWork {
// need to Define by user:
extern WBaseSession *CreateNewSession(WBaseSession::Listener *listener);
extern WNetWorkHandler *CreateNetworkHandlerAndInit(uint32_t events_size);

// need to override by user:
class WBaseSession;
}

#endif //UTILS_DEMO_UTILS_INCLUDE_WNETWORK_WNETWORK_H_
