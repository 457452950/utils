#pragma once
#ifndef UTILS_CHANNEL_H
#define UTILS_CHANNEL_H

#include <utility>

#include "wutils/network/base/Defined.h"

#include "wutils/Buffer.h"
#include "wutils/Error.h"
#include "wutils/SharedPtr.h"
#include "wutils/network/base/Timer.h"

namespace wutils::network::event {

/**************************************************
 * IOEvent interface
 ***************************************************/
class IOEvent {
public:
    IOEvent()          = default;
    virtual ~IOEvent() = default;

    // nocopy
    IOEvent(const IOEvent &other)            = delete;
    IOEvent &operator=(const IOEvent &other) = delete;

    virtual void EventIn()  = 0;
    virtual void EventOut() = 0;
};

class IOInOnly : public IOEvent {
public:
    IOInOnly()           = default;
    ~IOInOnly() override = default;

private:
    void EventOut() final{};
};

class IOOutOnly : public IOEvent {
public:
    IOOutOnly()           = default;
    ~IOOutOnly() override = default;

private:
    void EventIn() final{};
};


/* Impl */

//
///***********************************************************
// * UDPChannel
// ************************************************************/
// template <class FAMILY>
// class UDPChannel : public IOEvent {
// public:
//    explicit UDPChannel(weak_ptr<IO_Context> handle);
//    ~UDPChannel() override;
//
//    bool
//    Start(const typename FAMILY::EndPointInfo &local_ep, const typename FAMILY::EndPointInfo &remote_ep, bool shared);
//
//    // listener
// public:
//    class Listener {
//    public:
//        virtual void OnMessage(const uint8_t *message, uint64_t message_len) = 0;
//        virtual void OnError(SystemError)                                    = 0;
//    };
//
//    inline void SetListener(weak_ptr<Listener> listener) { this->listener_ = std::move(listener); }
//
// protected:
//    weak_ptr<Listener> listener_;
//
// public:
//    // unreliable
//    bool Send(const uint8_t *send_message, uint32_t message_len);
//
// private:
//    void EventIn() final;
//    void EventOut() final{};
//
//    void onErr(SystemError);
//
// protected:
//    IO_Handle_p                   handler_;
//    typename FAMILY::EndPointInfo local_endpoint_;
//    typename FAMILY::EndPointInfo remote_endpoint_;
//};
//
///**
// *  异步IO，参考 ASIO
// */
//// TODO: like asio
// class ASChannel : public IOEvent {
// public:
//     ASChannel(const EndPointInfo &, const EndPointInfo &, ev_hdler_p);
//     ~ASChannel() override;
//
//     bool Init() { return true; }
//     void ShutDown(int how);
//
//     const EndPointInfo &GetLocalInfo() { return local_endpoint_; }
//     const EndPointInfo &GetRemoteInfo() { return remote_endpoint_; }
//
// protected:
//     EndPointInfo local_endpoint_;
//     EndPointInfo remote_endpoint_;
//     ev_hdler_p   event_handler_;
//
// public:
//     struct ABuffer {
//         uint8_t *buffer{nullptr};
//         uint64_t buf_len{0};
//     };
//     void ARecv(ABuffer buffer);
//     void ASend(ABuffer buffer);
//
// protected:
//     ABuffer recv_buffer_;
//     ABuffer send_buffer_;
//
//     // listener
// public:
//     class Listener {
//     public:
//         virtual void onChannelDisConnect()     = 0;
//         virtual void onReceive(ABuffer buffer) = 0;
//         virtual void onSend(ABuffer buffer)    = 0;
//         virtual void onError(SystemError)      = 0;
//
//         virtual ~Listener() = default;
//     };
//
//     inline void SetListener(weak_ptr<Listener> listener) { this->listener_ = std::move(listener); }
//
// protected:
//     weak_ptr<Listener> listener_;
//
//     void EventIn() override;
//     void EventOut() override;
//     void onChannelClose();
//     void onChannelError(SystemError);
// };


} // namespace wutils::network::event


#endif // UTILS_CHANNEL_H