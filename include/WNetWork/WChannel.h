#pragma once
#ifndef UTILS_WCHANNEL_H
#define UTILS_WCHANNEL_H

#include <memory>


#include "WEpoll.h"
#include "WEvent.h"
#include "WNetWorkDef.h"
#include "WSelect.h"
#include "stdIOVec.h"


namespace wlb::network {


/**
 *
 */
class WBaseChannel;

using ev_hdle_t = WEventHandle<WBaseChannel>;
using ev_hdle_p = ev_hdle_t *;

using ev_hdler_t = ev_hdle_t::WEventHandler;
using ev_hdler_p = ev_hdler_t *;

/**************************************************
 * WBaseChannel interface
 ***************************************************/
class WBaseChannel {
public:
    WBaseChannel() {}
    virtual ~WBaseChannel() {}

    // nocopy
    WBaseChannel(const WBaseChannel &other)            = delete;
    WBaseChannel &operator=(const WBaseChannel &other) = delete;

    virtual void ChannelIn()  = 0;
    virtual void ChannelOut() = 0;
};

class ReadChannel : public WBaseChannel {
public:
    ReadChannel(){};
    ~ReadChannel() override {}

private:
    void ChannelOut() final{};
};

class WriteChannel : public WBaseChannel {
public:
    WriteChannel(){};
    ~WriteChannel() override {}

private:
    void ChannelIn() final{};
};


/* Impl */

/***********************************************************
 * WTimer
 ************************************************************/
class WTimer : public ReadChannel {
public:
    explicit WTimer(std::weak_ptr<ev_hdle_t> handle);
    ~WTimer() override;

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
    std::unique_ptr<ev_hdler_t> handler_{nullptr};
    bool                        active_{false};
};

/***********************************************************
 * WAccepterChannel
 ************************************************************/
class WChannel;
class WAccepterChannel : public ReadChannel {
public:
    explicit WAccepterChannel(std::weak_ptr<ev_hdle_t> handle);
    ~WAccepterChannel() override;

    bool Start(const WEndPointInfo &local_endpoint);

    using accept_cb = std::function<void(const WEndPointInfo &, const WEndPointInfo &, std::unique_ptr<ev_hdler_t>)>;
    accept_cb                         OnAccept;
    std::function<void(const char *)> OnError;

private:
    void ChannelIn() final;

private:
    std::unique_ptr<ev_hdler_t> handler_{nullptr};
    WEndPointInfo               local_endpoint_;
};


/***********************************************************
 * WUDP
 ************************************************************/
// HACK: 逻辑上 WUDP not "is a" WBaseChannel.
class WUDP : public WBaseChannel {
public:
    explicit WUDP(std::weak_ptr<ev_hdle_t> handle);
    ~WUDP() override;

    bool Start(const WEndPointInfo &local_endpoint);

    using message_cb = std::function<void(const WEndPointInfo &, const WEndPointInfo &, const uint8_t *, uint32_t)>;
    message_cb                        OnMessage;
    std::function<void(const char *)> OnError;

    // unreliable
    bool SendTo(const uint8_t *send_message, uint32_t message_len, const WEndPointInfo &remote);

private:
    void ChannelIn() final;
    void ChannelOut() final{};

    void onErr(int err);

private:
    std::unique_ptr<ev_hdler_t> handler_{nullptr};
    WEndPointInfo               local_endpoint_;
};

/***********************************************************
 * WUDPChannel
 ************************************************************/
class WUDPChannel : public WBaseChannel {
public:
    explicit WUDPChannel(std::weak_ptr<ev_hdle_t> handle);
    ~WUDPChannel() override;

    bool Start(const WEndPointInfo &local_ep, const WEndPointInfo &remote_ep);

    // listener
public:
    class Listener {
    public:
        virtual void OnMessage(const uint8_t *message, uint64_t message_len) = 0;
        virtual void OnError(const char *err_message)                        = 0;
    };

    inline void SetListener(std::weak_ptr<Listener> listener) { this->listener_ = listener; }

protected:
    std::weak_ptr<Listener> listener_;

public:
    // unreliable
    bool Send(const uint8_t *send_message, uint32_t message_len);

private:
    void ChannelIn() final;
    void ChannelOut() final{};

    void onErr(int err);

protected:
    std::unique_ptr<ev_hdler_t> handler_{nullptr};
    WEndPointInfo               local_endpoint_;
    WEndPointInfo               remote_endpoint_;
};

/*****************************************
 *  WChannel
 ******************************************/
class WChannel : public WBaseChannel {
public:
    explicit WChannel(const WEndPointInfo &, const WEndPointInfo &, std::unique_ptr<ev_hdler_t>);
    ~WChannel() override;

    bool Init();
    void ShutDown(int how); // Async
    void CloseChannel();    // Sync

    virtual void Send(const uint8_t *send_message, uint32_t message_len);

protected:
    WEndPointInfo               local_endpoint_;
    WEndPointInfo               remote_endpoint_;
    std::unique_ptr<ev_hdler_t> event_handler_{nullptr};

    // listener
public:
    class Listener {
    public:
        virtual void onChannelConnect(std::shared_ptr<WChannel>)             = 0;
        virtual void onChannelDisConnect()                                   = 0;
        virtual void onReceive(const uint8_t *message, uint64_t message_len) = 0;
        virtual void onError(const char *err_message)                        = 0;
    };

    inline void SetListener(std::weak_ptr<Listener> listener) { this->listener_ = listener; }

protected:
    std::weak_ptr<Listener> listener_;

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


} // namespace wlb::network


#endif // UTILS_WCHANNEL_H