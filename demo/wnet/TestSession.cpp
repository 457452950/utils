//
// Created by wang on 22-5-5.
//
#include "TestSession.h"


wlb::NetWork::WBaseSession *wlb::NetWork::CreateNewSession(WBaseSession::Listener *listener) {
    return new Session(listener);
}

wlb::NetWork::WNetWorkHandler *wlb::NetWork::CreateNetworkHandlerAndInit(uint32_t events_size) {
    auto c = new WEpoll();
    c->Init(events_size);
    return c;
}
