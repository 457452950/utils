#pragma once
#ifndef UTILS_CHANNEL_H
#define UTILS_CHANNEL_H

#include <utility>

#include "Epoll.h"
#include "Event.h"
#include "NetWorkDef.h"
#include "Select.h"
#include "stdIOVec.h"

#include "wutils/Buffer.h"
#include "wutils/SharedPtr.h"

namespace wutils::network {

/**
 *
 */
class BaseChannel;

using ev_hdle_t = EventHandle<BaseChannel>;
using ev_hdle_p = ev_hdle_t *;

using ev_hdler_t = ev_hdle_t::EventHandler;
using ev_hdler_p = shared_ptr<ev_hdler_t>;

void setCommonCallBack(ev_hdle_p handle);

/**************************************************
 * BaseChannel interface
 ***************************************************/
class BaseChannel {
public:
    BaseChannel()          = default;
    virtual ~BaseChannel() = default;

    // nocopy
    BaseChannel(const BaseChannel &other)            = delete;
    BaseChannel &operator=(const BaseChannel &other) = delete;

    virtual void ChannelIn()  = 0;
    virtual void ChannelOut() = 0;
};

class ReadChannel : public BaseChannel {
public:
    ReadChannel()           = default;
    ~ReadChannel() override = default;

private:
    void ChannelOut() final{};
};

class WriteChannel : public BaseChannel {
public:
    WriteChannel()           = default;
    ~WriteChannel() override = default;

private:
    void ChannelIn() final{};
};


/* Impl */

/***********************************************************
 * Timer
 ************************************************************/
class Timer : public ReadChannel {
public:
    explicit Timer(weak_ptr<ev_hdle_t> handle);
    ~Timer() override;

    std::function<void()> OnTime;

    // time_value 初次启动的计时时长，单位为ms,
    // interval循环定时器定时时长，单次定时器设置为0,默认采用相对时间
    bool Start(long time_value, long interval = 0);
    void Stop();
    // 定时器是否活跃，即是否已完成定时任务
    bool IsActive() const { return this->handler_->IsEnable(); }

private:
    void ChannelIn() final;

private:
    ev_hdler_p handler_;
};

/***********************************************************
 * AcceptorChannel
 ************************************************************/
class AcceptorChannel : public ReadChannel {
public:
    explicit AcceptorChannel(weak_ptr<ev_hdle_t> handle);
    ~AcceptorChannel() override;

    bool Start(const EndPointInfo &local_endpoint, bool shared);

    using accept_cb = std::function<void(const EndPointInfo &, const EndPointInfo &, ev_hdler_p)>;
    accept_cb                         OnAccept;
    std::function<void(const char *)> OnError;

private:
    void ChannelIn() final;

private:
    ev_hdler_p   handler_{nullptr};
    EndPointInfo local_endpoint_;
};


/***********************************************************
 * UDPPointer
 ************************************************************/
// XXX: 逻辑上 UDPPointer not "is a" BaseChannel.
class UDPPointer : public BaseChannel {
public:
    explicit UDPPointer(weak_ptr<ev_hdle_t> handle);
    ~UDPPointer() override;

    bool Start(const EndPointInfo &local_endpoint, bool shared);

    using message_cb = std::function<void(const EndPointInfo &, const EndPointInfo &, const uint8_t *, uint32_t)>;
    message_cb                        OnMessage;
    std::function<void(const char *)> OnError;

    // unreliable
    bool SendTo(const uint8_t *send_message, uint32_t message_len, const EndPointInfo &remote);

private:
    void ChannelIn() final;
    void ChannelOut() final{};

    void onErr(int err);

private:
    ev_hdler_p   handler_{nullptr};
    EndPointInfo local_endpoint_;
};

/***********************************************************
 * UDPChannel
 ************************************************************/
class UDPChannel : public BaseChannel {
public:
    explicit UDPChannel(weak_ptr<ev_hdle_t> handle);
    ~UDPChannel() override;

    bool Start(const EndPointInfo &local_ep, const EndPointInfo &remote_ep, bool shared);

    // listener
public:
    class Listener {
    public:
        virtual void OnMessage(const uint8_t *message, uint64_t message_len) = 0;
        virtual void OnError(const char *err_message)                        = 0;
    };

    inline void SetListener(weak_ptr<Listener> listener) { this->listener_ = std::move(listener); }

protected:
    weak_ptr<Listener> listener_;

public:
    // unreliable
    bool Send(const uint8_t *send_message, uint32_t message_len);

private:
    void ChannelIn() final;
    void ChannelOut() final{};

    void onErr(int err);

protected:
    ev_hdler_p   handler_;
    EndPointInfo local_endpoint_;
    EndPointInfo remote_endpoint_;
};

/*****************************************
 *  Channel
 ******************************************/
class Channel : public BaseChannel {
public:
    explicit Channel(const EndPointInfo &, const EndPointInfo &, ev_hdler_p);
    ~Channel() override;

    bool Init();
    void ShutDown(int how); // Async

    virtual void Send(const uint8_t *send_message, uint32_t message_len);

    const EndPointInfo &GetLocalInfo() { return local_endpoint_; }
    const EndPointInfo &GetRemoteInfo() { return remote_endpoint_; }

protected:
    EndPointInfo local_endpoint_;
    EndPointInfo remote_endpoint_;
    ev_hdler_p   event_handler_;

    // listener
public:
    class Listener {
    public:
        // virtual void onChannelConnect(std::shared_ptr<Channel>)             = 0;
        virtual void onChannelDisConnect()                                   = 0;
        virtual void onReceive(const uint8_t *message, uint64_t message_len) = 0;
        virtual void onError(const char *err_message)                        = 0;

        virtual ~Listener() = default;
    };

    inline void SetListener(weak_ptr<Listener> listener) { this->listener_ = std::move(listener); }

protected:
    weak_ptr<Listener> listener_;

    // buffer
public:
    uint64_t GetRecvBufferSize() const { return max_recv_buf_size_; }
    uint64_t GetSendBufferSize() const { return max_send_buf_size_; }
    void     SetRecvBufferMaxSize(uint64_t max_size);
    void     SetSendBufferMaxSize(uint64_t max_size);

private:
    // receive buffer
    shared_ptr<RingBuffer> recv_buf{new RingBuffer()};
    uint64_t               max_recv_buf_size_{0};
    // send buffer
    shared_ptr<RingBuffer> send_buf{new RingBuffer()};
    uint64_t               max_send_buf_size_{0};

protected:
    // can override
    void ChannelIn() override;
    void ChannelOut() override;
    void onChannelClose();
    void onChannelError(int error_code);
};

/**
 *  异步IO，参考 ASIO
 */
// TODO: like asio
class ASChannel : public BaseChannel {
public:
    ASChannel(const EndPointInfo &, const EndPointInfo &, ev_hdler_p);
    ~ASChannel() override;

    bool Init() { return true; }
    void ShutDown(int how);

    const EndPointInfo &GetLocalInfo() { return local_endpoint_; }
    const EndPointInfo &GetRemoteInfo() { return remote_endpoint_; }

protected:
    EndPointInfo local_endpoint_;
    EndPointInfo remote_endpoint_;
    ev_hdler_p   event_handler_;

public:
    struct ABuffer {
        uint8_t *buffer{nullptr};
        uint64_t buf_len{0};
    };
    void ARecv(ABuffer buffer);
    void ASend(ABuffer buffer);

protected:
    ABuffer recv_buffer_;
    ABuffer send_buffer_;

    // listener
public:
    class Listener {
    public:
        virtual void onChannelDisConnect()            = 0;
        virtual void onReceive(ABuffer buffer)        = 0;
        virtual void onSend(ABuffer buffer)           = 0;
        virtual void onError(const char *err_message) = 0;

        virtual ~Listener() = default;
    };

    inline void SetListener(weak_ptr<Listener> listener) { this->listener_ = std::move(listener); }

protected:
    weak_ptr<Listener> listener_;

    void ChannelIn() override;
    void ChannelOut() override;
    void onChannelClose();
    void onChannelError(int error_code);
};


} // namespace wutils::network


#endif // UTILS_CHANNEL_H