#pragma once
#ifndef UTILS_CHANNEL_H
#define UTILS_CHANNEL_H

namespace wutils::network::event {

// TODO: better name
/**
 *
 */
class IOEvent {
public:
    IOEvent()          = default;
    virtual ~IOEvent() = default;

    // nocopy
    IOEvent(const IOEvent &other)            = delete;
    IOEvent &operator=(const IOEvent &other) = delete;

    virtual void IOIn()  = 0;
    virtual void IOOut() = 0;
};

class IOReadEvent : public IOEvent {
public:
    IOReadEvent()           = default;
    ~IOReadEvent() override = default;

private:
    void IOOut() final{};
};

class IOWriteEvent : public IOEvent {
public:
    IOWriteEvent()           = default;
    ~IOWriteEvent() override = default;

private:
    void IOIn() final{};
};

//
///* Impl */
//
///*****************************************
// *  Channel
// ******************************************/
// class Channel : public IOEvent {
// public:
//    explicit Channel(const NetAddress &, const NetAddress &, io_hdle_p);
//    ~Channel() override;
//
//    bool Init();
//    void ShutDown(int how); // Async
//
//    virtual void Send(const uint8_t *send_message, uint32_t message_len);
//
//    const NetAddress &GetLocalAddress() { return local_endpoint_; }
//    const NetAddress &GetRemoteAddress() { return remote_endpoint_; }
//
// protected:
//    NetAddress     local_endpoint_;
//    NetAddress     remote_endpoint_;
//    io_hdle_p    event_handler_;
//
//    // listener
// public:
//    class Listener {
//    public:
//        // virtual void onChannelConnect(std::shared_ptr<Channel>)             = 0;
//        virtual void onChannelDisConnect()                                   = 0;
//        virtual void onReceive(const uint8_t *message, uint64_t message_len) = 0;
//        virtual void onError(SystemError)                                    = 0;
//
//        virtual ~Listener() = default;
//    };
//
//    inline void SetListener(weak_ptr<Listener> listener) { this->listener_ = std::move(listener); }
//
// protected:
//    weak_ptr<Listener> listener_;
//
//    // buffer
// public:
//    uint64_t GetRecvBufferSize() const { return max_recv_buf_size_; }
//    uint64_t GetSendBufferSize() const { return max_send_buf_size_; }
//    void     SetRecvBufferMaxSize(uint64_t max_size);
//    void     SetSendBufferMaxSize(uint64_t max_size);
//
// private:
//    // receive buffer
//    shared_ptr<RingBuffer> recv_buf_{new RingBuffer()};
//    uint64_t               max_recv_buf_size_{0};
//    // send buffer
//    shared_ptr<RingBuffer> send_buf_{new RingBuffer()};
//    uint64_t               max_send_buf_size_{0};
//
// protected:
//    // can override
//    void IOIn() override;
//    void IOOut() override;
//    void onChannelClose();
//    void onChannelError(SystemError);
//};
//
///**
// *  异步IO，参考 ASIO
// */
//// TODO: like asio
// class ASChannel : public IOEvent {
// public:
//     ASChannel(const NetAddress &, const NetAddress &, io_hdle_p);
//     ~ASChannel() override;
//
//     bool Init() { return true; }
//     void ShutDown(int how);
//
//     const NetAddress &GetLocalAddress() { return local_endpoint_; }
//     const NetAddress &GetRemoteAddress() { return remote_endpoint_; }
//
// protected:
//     NetAddress     local_endpoint_;
//     NetAddress     remote_endpoint_;
//     io_hdle_p    event_handler_;
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
//     void IOIn() override;
//     void IOOut() override;
//     void onChannelClose();
//     void onChannelError(SystemError);
// };


} // namespace wutils::network::event


#endif // UTILS_CHANNEL_H