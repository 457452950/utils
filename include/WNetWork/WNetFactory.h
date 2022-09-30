#pragma once
#ifndef UTILS_WNETFACTORY_H
#define UTILS_WNETFACTORY_H

#include <memory>


#include "WEpoll.h"
#include "WSelect.h"

#include "WBaseSession.h"
#include "WChannel.h"
#include "WNetWorkDef.h"

namespace wlb::network {


class WChannelFactory;
class WSessionFactory;

struct EventContext {
    // accepter
    // return true for accpet the connection
    using accept_cb_t       = bool (*)(base_socket_type socket, WEndPointInfo &endpoint);
    using accept_error_cb_t = void (*)(int error_no);

    //
    accept_cb_t       onAccept{nullptr};
    accept_error_cb_t onAcceptError{nullptr};
    //
    event_handle_t *event_handle_{nullptr};
    //
    WChannelFactory *channel_factory_{nullptr};
    WSessionFactory *session_factory_{nullptr};
};


namespace WNetFactory {

inline event_handle_p CreateNetHandle(HandleType type) {
    switch(type) {
    case HandleType::SELECT:
        return new WSelect<WBaseChannel>();

    case HandleType::EPOLL:
        return new WEpoll<WBaseChannel>();

    default:
        abort();
    }
    return nullptr;
}

}; // namespace WNetFactory


/**
 * WChannelFactory 工厂
 */
class WChannelFactory {
public:
    virtual std::unique_ptr<WChannel> CreateChannel() { return std::make_unique<WChannel>(this->buffer_size_); };

    void SetChannelBufferSize(uint16_t size) noexcept { this->buffer_size_ = size; }

public:
    WChannelFactory() {}
    virtual ~WChannelFactory() {}

private:
    uint16_t buffer_size_{1024};
};

/**
 * WSessionFactory
 */
class WSessionFactory {
public:
    virtual std::shared_ptr<WBaseSession> CreateSession(std::unique_ptr<WChannel> channel) = 0;

public:
    WSessionFactory() {}
    virtual ~WSessionFactory() {}
};

} // namespace wlb::network

#endif
