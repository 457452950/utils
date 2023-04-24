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
    explicit WTimer(event_handle_p handle);
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
    event_handler_p handler_{nullptr};
    bool            active_{false};
};

/***********************************************************
 * WAccepterChannel
 ************************************************************/
class WChannel;
class WAccepterChannel : public ReadChannel {
public:
    explicit WAccepterChannel(const WEndPointInfo &local_endpoint, event_handle_p handle);
    ~WAccepterChannel() override;

    std::function<WBaseChannel *(WEndPointInfo, WEndPointInfo, event_handler_p)> OnAccept;
    std::function<void(const char *)>                                            OnError;

private:
    void ChannelIn() final;

private:
    event_handler_p handler_{nullptr};
    WEndPointInfo   local_endpoint_;
};

/***********************************************************
 * WUDPChannel
 ************************************************************/
class WUDPChannel : public WBaseChannel {
public:
    explicit WUDPChannel(const WEndPointInfo &local_endpoint, event_handle_p handle);
    ~WUDPChannel() override;

    std::function<void(WEndPointInfo, WEndPointInfo, uint8_t *, uint32_t, event_handler_p)> OnMessage;
    std::function<void(const char *)>                                                       OnError;

    // TODO:Send
    //

private:
    void ChannelIn() final;
    void ChannelOut() final;

private:
    event_handler_p handler_{nullptr};
    WEndPointInfo   local_endpoint_;
};


/**
 * 链接状态
 */
enum class WChannelState {
    /**
     * 链接关闭
     */
    CLOSE = 0,
    /**
     * 链接
     */
    CONNECT = 1,
};

class WChannel : public WBaseChannel {
public:
    explicit WChannel(WEndPointInfo, WEndPointInfo, event_handler_p);
    ~WChannel() override;

    bool Init();
    void ShutDown(int how); // Async
    void CloseChannel();    // Sync

    virtual void Send(uint8_t *send_message, uint64_t message_len);

protected:
    WChannelState   channelState_{WChannelState::CLOSE};
    WEndPointInfo   local_endpoint_;
    WEndPointInfo   remote_endpoint_;
    event_handler_p event_handler_{nullptr};

    // listener
public:
    class Listener {
    public:
        virtual void onChannelConnect()                                = 0;
        virtual void onChannelDisConnect()                             = 0;
        virtual void onReceive(uint8_t *message, uint64_t message_len) = 0;
        virtual void onError(const char *err_message)                  = 0;
    };

    inline void SetListener(Listener *listener) { this->listener_ = listener; }

protected:
    Listener *listener_{nullptr};

    // buffer
public:
    uint64_t GetRecvBufferSize() const { return recv_buf_size_; }
    uint64_t GetSendBufferSize() const { return send_buf_size_; }
    void     SetRecvBufferMaxSize(uint64_t);
    void     SetSendBufferMaxSize(uint64_t);

private:
    void FreeRecvBuffer();
    void FreeSendBuffer();
    // receive buffer
    uint8_t *recv_buf_{nullptr};
    uint64_t recv_buf_size_{0};
    // send buffer
    uint8_t *send_buf_{nullptr};
    uint64_t send_buf_size_{0};
    uint64_t send_len_{0}; // 发送的字节长度

protected:
    // can override
    void ChannelIn() override;
    void ChannelOut() override;
    void onChannelClose();
    void onChannelError(uint64_t error_code);
};


} // namespace wlb::network


#endif // UTILS_WCHANNEL_H