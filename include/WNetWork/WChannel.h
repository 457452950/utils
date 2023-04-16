#pragma once
#ifndef UTILS_WCHANNEL_H
#define UTILS_WCHANNEL_H

#include "WEpoll.h"
#include "WEvent.h"
#include "WSelect.h"


namespace wlb::network {


class WBaseChannel {
public:
    virtual ~WBaseChannel() {}

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

class WTimer : public ReadChannel {
public:
    explicit WTimer(event_handle_p handle);
    ~WTimer() override;

    typedef void (*on_time_cb_t)(void);
    on_time_cb_t OnTime{nullptr};

    // time_value 初次启动的计时时长，单位为ms, interval循环定时器定时时长，单次定时器设置为0,默认采用相对时间
    bool Start(long time_value, long interval = 0);
    void Stop();
    // 定时器是否活跃，即是否已完成定时任务
    inline bool IsActive() const { return this->active_; }

private:
    void ChannelIn() final;

private:
    event_handle_p                   handle_{nullptr};
    event_handle_t::option_list_item item_;
    timerfd_t                        timer_fd_{-1};
    bool                             active_{false};
};

class WAccepterChannel : public ReadChannel {
public:
    explicit WAccepterChannel(base_socket_type socket, const WEndPointInfo &local_endpoint, event_context_p context);
    ~WAccepterChannel() override;

private:
    void ChannelIn() final;

private:
    base_socket_type listen_socket_{-1};
    WEndPointInfo    local_endpoint_;
    event_context_p  event_context_{nullptr};
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
    explicit WChannel(uint16_t buffer_size);
    ~WChannel() override;
    // nocopy
    WChannel(const WChannel &other)            = delete;
    WChannel &operator=(const WChannel &other) = delete;

    class Listener {
    public:
        virtual void onChannelDisConnect()                             = 0;
        virtual void onReceive(uint8_t *message, uint64_t message_len) = 0;
        virtual void onError(uint64_t err_code)                        = 0;
    };

    void        Init(base_socket_type socket, const WEndPointInfo &remote_endpoint);
    inline void SetListener(Listener *listener) { this->listener_ = listener; }
    void        SetEventHandle(event_handle_p handle);
    void        CloseChannel(); // Async

    virtual void Send(void *send_message, uint64_t message_len);

protected:
    // can override
    virtual void ChannelIn();
    virtual void ChannelOut();

    void onChannelClose();
    void onChannelError(uint64_t error_code);

private:
    // native socket
    WChannelState                    channelState_{WChannelState::CLOSE};
    base_socket_type                 client_socket_{-1};
    WEndPointInfo                    remote_endpoint_;
    event_handle_p                   event_handle_{nullptr};
    event_handle_t::option_list_item option_item_;
    // listener
    Listener *listener_{nullptr};
    // receive buffer
    uint8_t *recv_buf_{nullptr};
    uint64_t recv_buf_size_{0};
    // send buffer
    uint8_t *send_buf_{nullptr};
    uint64_t send_buf_size_{0};
    uint64_t send_size_{0};
};


} // namespace wlb::network


#endif //UTILS_WCHANNEL_H