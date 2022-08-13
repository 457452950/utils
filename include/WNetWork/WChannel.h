#pragma once

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
    event_handle_p               handle_{nullptr};
    event_handle_t::fd_list_item item_;
    timerfd_t                    timer_fd_{-1};
    bool                         active_{false};
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

class WChannel : public WBaseChannel {
public:
    explicit WChannel(base_socket_type socket, WEndPointInfo &remote_endpoint, event_context_p context);
    ~WChannel() override;
    // nocopy
    WChannel(const WChannel &other)            = delete;
    WChannel &operator=(const WChannel &other) = delete;

    void Send(void *send_message, uint64_t message_len);

private:
    void ChannelIn() final;
    void ChannelOut() final;

private:
    base_socket_type                         client_socket_{-1};
    WEndPointInfo                            remote_endpoint_;
    event_context_p                          event_context_{nullptr};
    event_handle_t::fd_list_item             item_;

    uint8_t *recv_buf_{nullptr};
    uint64_t recv_buf_size_{0};
    uint8_t *send_buf_{nullptr};
    uint64_t send_buf_size_{0};
    uint64_t send_size_{0};
};


struct EventContext {
    // accepter
    // return true for accpet the connection
    using accept_cb_t       = bool (*)(base_socket_type socket, WEndPointInfo &endpoint);
    using accept_error_cb_t = void (*)(int error_no); 
    // consumer channel
    using read_cb_t         = void (*)(WChannel* channel, void* read_data, int64_t read_size);
    using read_error_cb_t   = void (*)(WChannel* Channel, int error_no);
    using write_error_cb_t  = read_error_cb_t; 

    accept_cb_t         onAccept{nullptr};
    accept_error_cb_t   onAcceptError{nullptr};

    read_cb_t           onRead{nullptr};
    read_error_cb_t     onReadError{nullptr};
    write_error_cb_t    onWriteError{nullptr};

    uint64_t                    max_read_size_{0};
    event_handle_t             *event_handle_{nullptr};
};


} // namespace wlb::network
