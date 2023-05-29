#pragma once
#ifndef UTILS_CHANNEL_H
#define UTILS_CHANNEL_H

#include <utility>

#include "Epoll.h"
#include "Event.h"
#include "NetWorkDef.h"
#include "Select.h"
#include "stdIOVec.h"

#include "wutils/SharedPtr.h"

namespace wutils::network {

/**
 *
 */
class BaseChannel;

using ev_hdle_t = EventHandle<BaseChannel>;
using ev_hdle_p = ev_hdle_t *;

using ev_hdler_t = ev_hdle_t::EventHandler;
using ev_hdler_p = ev_hdler_t *;

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
    bool IsActive() const { return this->active_; }

private:
    void ChannelIn() final;

private:
    unique_ptr<ev_hdler_t> handler_{nullptr};
    bool                   active_{false};
};

/***********************************************************
 * AcceptorChannel
 ************************************************************/
class AcceptorChannel : public ReadChannel {
public:
    explicit AcceptorChannel(weak_ptr<ev_hdle_t> handle);
    ~AcceptorChannel() override;

    bool Start(const EndPointInfo &local_endpoint, bool shared);

    using accept_cb = std::function<void(const EndPointInfo &, const EndPointInfo &, unique_ptr<ev_hdler_t>)>;
    accept_cb                         OnAccept;
    std::function<void(const char *)> OnError;

private:
    void ChannelIn() final;

private:
    unique_ptr<ev_hdler_t> handler_{nullptr};
    EndPointInfo           local_endpoint_;
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
    unique_ptr<ev_hdler_t> handler_{nullptr};
    EndPointInfo           local_endpoint_;
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
    unique_ptr<ev_hdler_t> handler_{nullptr};
    EndPointInfo           local_endpoint_;
    EndPointInfo           remote_endpoint_;
};

/*****************************************
 *  Channel
 ******************************************/
class Channel : public BaseChannel {
public:
    explicit Channel(const EndPointInfo &, const EndPointInfo &, unique_ptr<ev_hdler_t>);
    ~Channel() override;

    bool Init();
    void ShutDown(int how); // Async
    void CloseChannel();    // TODO: Sync

    virtual void Send(const uint8_t *send_message, uint32_t message_len);

    const EndPointInfo &GetLocalInfo() { return local_endpoint_; }
    const EndPointInfo &GetRemoteInfo() { return remote_endpoint_; }

protected:
    EndPointInfo           local_endpoint_;
    EndPointInfo           remote_endpoint_;
    unique_ptr<ev_hdler_t> event_handler_{nullptr};

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
    void     SetRecvBufferMaxSize(int page_count, int page_size);
    void     SetSendBufferMaxSize(int page_count, int page_size);

private:
    // receive buffer
    IOVec    recv_buf;
    uint64_t max_recv_buf_size_{0};
    // send buffer
    IOVec    send_buf;
    uint64_t max_send_buf_size_{0};

protected:
    // can override
    void ChannelIn() override;
    void ChannelOut() override;
    void onChannelClose();
    void onChannelError(uint64_t error_code);
};


} // namespace wutils::network


#endif // UTILS_CHANNEL_H