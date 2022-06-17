#pragma once

#include "WEpoll.h"
#include "WEvent.h"
#include "WSelect.h"


namespace wlb::network {

class WBaseChannel {
public:
    virtual ~WBaseChannel() {}
};

class ReadChannel : virtual public WBaseChannel {
public:
    ReadChannel(){};
    ~ReadChannel() override {}

    virtual void ChannelIn() = 0;
};

class WriteChannel : virtual public WBaseChannel {
public:
    WriteChannel(){};
    ~WriteChannel() override {}

    virtual void ChannelOut() = 0;
};

class WTimer : public ReadChannel {
public:
    explicit WTimer(WEventHandle *handle);
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
    WEventHandle              *handle_{nullptr};
    WEventHandle::fd_list_item item_;
    timerfd_t                  timer_fd_{-1};
    bool                       active_{false};
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

class WChannel : public ReadChannel, public WriteChannel {
public:
    explicit WChannel(base_socket_type socket, WEndPointInfo &remote_endpoint, event_context_p context);
    ~WChannel() override;
    // nocopy
    WChannel(const WChannel &other) = delete;
    WChannel &operator = (const WChannel &other) = delete;

    void Send(void *send_message, uint64_t message_len);

private:
    void ChannelIn() final;
    void ChannelOut() final;

private:
    base_socket_type           client_socket_{-1};
    WEndPointInfo              remote_endpoint_;
    event_context_p            event_context_{nullptr};
    WEventHandle::fd_list_item item_;

    uint8_t *recv_buf_{nullptr};
    uint64_t recv_buf_size_{0};
    uint8_t *send_buf_{nullptr};
    uint64_t send_buf_size_{0};
    uint64_t send_size_{0};
};


struct EventContext {
    // accepter
    // return true for accpet the connection
    typedef bool (*accept_cb_t)(base_socket_type socket, WEndPointInfo &endpoint);
    typedef void (*accept_error_cb_t)(int error_no);
    // consumer channel
    typedef void (*read_cb_t)(WChannel *channel, void *read_data, int64_t read_size);
    typedef void (*read_error_cb_t)(ReadChannel *channel, int error_no);
    typedef void (*write_error_cb_t)(WriteChannel *channel, int error_no);

    accept_cb_t       onAccept{nullptr};
    accept_error_cb_t onAcceptError{nullptr};

    read_cb_t        onRead{nullptr};
    read_error_cb_t  onReadError{nullptr};
    write_error_cb_t onWriteError{nullptr};

    uint64_t      max_read_size_{0};
    WEventHandle *event_handle_{nullptr};
};


} // namespace wlb::network
