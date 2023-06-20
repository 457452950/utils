#pragma once
#ifndef UTILS_NETFACTORY_H
#define UTILS_NETFACTORY_H

#include <memory>


#include "Epoll.h"
#include "Select.h"

#include "Channel.h"
#include "NetworkDef.h"

namespace wutils::network {


namespace NetFactory {

inline shared_ptr<ev_hdle_t> CreateNetHandle(HandleType type) {
    switch(type) {
    case HandleType::SELECT:
        return make_shared<Select<BaseChannel>>();

    case HandleType::EPOLL:
        return make_shared<Epoll<BaseChannel>>();

    default:
        abort();
    }
    return nullptr;
}

inline shared_ptr<ev_hdle_t> CreateDefaultNetHandle() { return CreateNetHandle(default_handle_type); }

inline shared_ptr<Channel>
CreateDefaultChannel(const EndPointInfo &local, const EndPointInfo &remote, unique_ptr<ev_hdler_t> handler) {
    return make_shared<Channel>(local, remote, std::move(handler));
}

}; // namespace NetFactory

template <typename Channel>
struct TCPEvFactory {
    // clang-format off
    template<typename channel>
    using newChannel = std::function<shared_ptr<channel>(
                                                const EndPointInfo &,
                                                const EndPointInfo &,
                                                shared_ptr<typename channel::Listener>, 
                                                unique_ptr<ev_hdler_t>)>;

    newChannel<Channel> NewChannel = NetFactory::CreateDefaultChannel;

    using newSession = std::function<shared_ptr<typename Channel::Listener>(
                                                    const EndPointInfo &,
                                                    const EndPointInfo &)>;
    newSession NewSession;
    // clang-format on
};


} // namespace wutils::network

#endif
