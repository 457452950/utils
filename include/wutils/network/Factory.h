#pragma once
#ifndef UTILS_NETFACTORY_H
#define UTILS_NETFACTORY_H

#include <memory>

#include "IO_Event.h"
#include "NetWork.h"
#include "TcpConnection.h"
#include "wutils/network/base/Defined.h"

namespace wutils::network {


namespace NetFactory {

inline shared_ptr<event::IO_Context> CreateNetHandle(HandleType type) {
    switch(type) {
    case HandleType::SELECT:
        return make_shared<event::SelectContext>();

    case HandleType::EPOLL:
        return make_shared<event::EpollContext>();

    default:
        abort();
    }
    return nullptr;
}

inline shared_ptr<event::IO_Context> DefaultContext() { return CreateNetHandle(default_handle_type); }

template <class FAMILY>
inline auto CreateDefaultChannel(const typename FAMILY::EndPointInfo &local,
                                 const typename FAMILY::EndPointInfo &remote,
                                 event::IO_Context::IO_Handle_p       handler) {
    return make_shared<event::Channel<FAMILY>>(local, remote, std::move(handler));
}

}; // namespace NetFactory


} // namespace wutils::network

#endif
