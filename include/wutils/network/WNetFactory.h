#pragma once
#ifndef UTILS_WNETFACTORY_H
#define UTILS_WNETFACTORY_H

#include <memory>


#include "WEpoll.h"
#include "WSelect.h"

#include "WBaseSession.h"
#include "WChannel.h"
#include "WNetWorkDef.h"

namespace wutils::network {


namespace WNetFactory {

inline std::shared_ptr<ev_hdle_t> CreateNetHandle(HandleType type) {
    switch(type) {
    case HandleType::SELECT:
        return std::make_shared<WSelect<WBaseChannel>>();

    case HandleType::EPOLL:
        return std::make_shared<WEpoll<WBaseChannel>>();

    default:
        abort();
    }
    return nullptr;
}

inline std::shared_ptr<ev_hdle_t> CreateDefaultNetHandle() { return CreateNetHandle(default_handle_type); }

inline std::shared_ptr<WChannel>
CreateDefaultChannel(const WEndPointInfo &local, const WEndPointInfo &remote, std::unique_ptr<ev_hdler_t> handler) {
    return std::make_shared<WChannel>(local, remote, std::move(handler));
}

}; // namespace WNetFactory

template <typename Channel>
struct WTCPEvFactory {
    // clang-format off
    template<typename channel>
    using newChannel = std::function<std::shared_ptr<channel>(
                                                const WEndPointInfo &, 
                                                const WEndPointInfo &,
                                                std::shared_ptr<typename channel::Listener>, 
                                                std::unique_ptr<ev_hdler_t>)>;

    newChannel<Channel> NewChannel = WNetFactory::CreateDefaultChannel;

    using newSession = std::function<std::shared_ptr<WChannel::Listener>(
                                                    const WEndPointInfo &, 
                                                    const WEndPointInfo &)>;
    newSession NewSession;
    // clang-format on
};


} // namespace wutils::network

#endif
