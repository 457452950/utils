#pragma once
#ifndef UTILS_CHANNEL_H
#define UTILS_CHANNEL_H

#include <utility>

#include "Defined.h"
#include "EndPoint.h"
#include "Epoll.h"
#include "IOContext.h"
#include "Select.h"

#include "wutils/Buffer.h"
#include "wutils/Error.h"
#include "wutils/SharedPtr.h"

namespace wutils::network {

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

using io_context_t = IOContext<IOEvent>;

using io_hdle_t = io_context_t::IOHandle;
using io_hdle_p = shared_ptr<io_hdle_t>;

void setCommonCallBack(io_context_t *ioContext);


/* Impl */

/***********************************************************
 * Timer
 ************************************************************/
class Timer : public IOReadEvent {
public:
    explicit Timer(weak_ptr<io_context_t> handle);
    ~Timer() override;

    std::function<void()> OnTime;

    // time_ms 初次启动的计时时长，单位为ms,
    // interval循环定时器定时时长，单次定时器设置为0,默认采用相对时间
    bool Start(long time_ms, long interval_ms);
    bool StartOnce(long time_ms);
    bool StartLoop(long interval_ms);
    void Stop();
    // 定时器是否活跃，即是否已完成定时任务
    bool IsActive() const { return this->handler_->IsEnable(); }

private:
    void IOIn() final;

private:
    io_hdle_p handler_;
};


/*****************************************
 *  Channel
 ******************************************/
class Channel : public IOEvent {
public:
    explicit Channel(const EndPointInfo &, const EndPointInfo &, io_hdle_p);
    ~Channel() override;

    bool Init();
    void ShutDown(int how); // Async

    virtual void Send(const uint8_t *send_message, uint32_t message_len);

    const EndPointInfo &GetLocalInfo() { return local_endpoint_; }
    const EndPointInfo &GetRemoteInfo() { return remote_endpoint_; }

protected:
    EndPointInfo local_endpoint_;
    EndPointInfo remote_endpoint_;
    io_hdle_p    event_handler_;

    // listener
public:
    class Listener {
    public:
        // virtual void onChannelConnect(std::shared_ptr<Channel>)             = 0;
        virtual void onChannelDisConnect()                                   = 0;
        virtual void onReceive(const uint8_t *message, uint64_t message_len) = 0;
        virtual void onError(SystemError)                                    = 0;

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
    shared_ptr<RingBuffer> recv_buf_{new RingBuffer()};
    uint64_t               max_recv_buf_size_{0};
    // send buffer
    shared_ptr<RingBuffer> send_buf_{new RingBuffer()};
    uint64_t               max_send_buf_size_{0};

protected:
    // can override
    void IOIn() override;
    void IOOut() override;
    void onChannelClose();
    void onChannelError(SystemError);
};

/**
 *  异步IO，参考 ASIO
 */
// TODO: like asio
class ASChannel : public IOEvent {
public:
    ASChannel(const EndPointInfo &, const EndPointInfo &, io_hdle_p);
    ~ASChannel() override;

    bool Init() { return true; }
    void ShutDown(int how);

    const EndPointInfo &GetLocalInfo() { return local_endpoint_; }
    const EndPointInfo &GetRemoteInfo() { return remote_endpoint_; }

protected:
    EndPointInfo local_endpoint_;
    EndPointInfo remote_endpoint_;
    io_hdle_p    event_handler_;

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
        virtual void onChannelDisConnect()     = 0;
        virtual void onReceive(ABuffer buffer) = 0;
        virtual void onSend(ABuffer buffer)    = 0;
        virtual void onError(SystemError)      = 0;

        virtual ~Listener() = default;
    };

    inline void SetListener(weak_ptr<Listener> listener) { this->listener_ = std::move(listener); }

protected:
    weak_ptr<Listener> listener_;

    void IOIn() override;
    void IOOut() override;
    void onChannelClose();
    void onChannelError(SystemError);
};


} // namespace wutils::network


#endif // UTILS_CHANNEL_H