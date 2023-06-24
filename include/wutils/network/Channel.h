#pragma once
#ifndef UTILS_CHANNEL_H
#define UTILS_CHANNEL_H

#include <utility>

#include "Epoll.h"
#include "Event.h"
#include "NetworkDef.h"
#include "Select.h"
#include "stdIOVec.h"

#include "Timer.h"
#include "wutils/Buffer.h"
#include "wutils/Error.h"
#include "wutils/SharedPtr.h"

namespace wutils::network::channel {

/**
 *
 */
class BaseChannel;

using IO_Context = event::IO_Context<BaseChannel>;

using IO_Handle_t = IO_Context::IO_Handle;
using IO_Handle_p = shared_ptr<IO_Handle_t>;

void setCommonCallBack(IO_Handle_t *handle);

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
    explicit Timer(weak_ptr<IO_Context> handle);
    ~Timer() override;

    std::function<void()> OnTime;

    template <typename Rep, typename Period>
    bool Once(const std::chrono::duration<Rep, Period> &rtime);
    template <typename Rep, typename Period>
    bool Repeat(const std::chrono::duration<Rep, Period> &rtime);
    void Stop();
    // 定时器是否活跃，即是否已完成定时任务
    bool IsActive() const { return this->handler_->IsEnable(); }

private:
    void ChannelIn() final;

private:
    IO_Handle_p   handler_;
    timer::Socket timer_socket_;
};

/***********************************************************
 * AcceptorChannel
 ************************************************************/
template <class FAMILY, class PROTOCOL>
class AcceptorChannel : public ReadChannel {
public:
    explicit AcceptorChannel(weak_ptr<IO_Context> context);
    ~AcceptorChannel() override;

    std::tuple<bool, SystemError> Start(const typename FAMILY::EndPointInfo &local_endpoint, bool shared);

    using accept_cb = std::function<void(
            const typename FAMILY::EndPointInfo &, const typename FAMILY::EndPointInfo &, IO_Handle_p)>;
    accept_cb                        OnAccept;
    std::function<void(SystemError)> OnError;

private:
    void ChannelIn() final;

private:
    IO_Handle_p                   handler_{nullptr};
    typename PROTOCOL::Acceptor   acceptor_;
    typename FAMILY::EndPointInfo local_endpoint_;
};

template <class FAMILY, class PROTOCOL>
AcceptorChannel<FAMILY, PROTOCOL>::AcceptorChannel(weak_ptr<IO_Context> context) {
    this->handler_             = make_shared<IO_Handle_t>();
    this->handler_->socket_    = this->acceptor_->GetNativeSocket();
    this->handler_->handle_    = std::move(context);
    this->handler_->user_data_ = this;
}
template <class FAMILY, class PROTOCOL>
AcceptorChannel<FAMILY, PROTOCOL>::~AcceptorChannel() {
    if(this->handler_) {
        if(this->handler_->IsEnable())
            this->handler_->DisEnable();
    }
}
template <class FAMILY, class PROTOCOL>
std::tuple<bool, SystemError>
AcceptorChannel<FAMILY, PROTOCOL>::Start(const typename FAMILY::EndPointInfo &local_endpoint, bool shared) {
    this->local_endpoint_ = local_endpoint;

    if(!this->acceptor_->Bind(local_endpoint)) {
        return {false, SystemError::GetSysErrCode()};
    }
    if(!this->acceptor_->Listen()) {
        return {false, SystemError::GetSysErrCode()};
    }

    this->handler_->socket_ = socket;
    this->handler_->SetEvents(event::EventType::EV_IN);
    this->handler_->Enable();

    return {true, {0}};
}

template <class FAMILY, class PROTOCOL>
void AcceptorChannel<FAMILY, PROTOCOL>::ChannelIn() {
    assert(this->handler_);

    typename FAMILY::EndPointInfo ei;

    auto cli = this->acceptor_.Accept(ei);

    if(cli <= 0) { // error
        if(OnError) {
            OnError(SystemError::GetSysErrCode());
        }
    } else {
        if(!OnAccept) {
            ::close(cli);
            return;
        }

        auto handler     = make_shared<IO_Handle_t>();
        handler->socket_ = cli;
        handler->handle_ = this->handler_->handle_;

        OnAccept(this->acceptor_.GetLocalInfo(), ei, handler);
    }
}


/***********************************************************
 * UDPChannel
 ************************************************************/
template <class FAMILY>
class UDPChannel : public BaseChannel {
public:
    explicit UDPChannel(weak_ptr<IO_Context> handle);
    ~UDPChannel() override;

    bool
    Start(const typename FAMILY::EndPointInfo &local_ep, const typename FAMILY::EndPointInfo &remote_ep, bool shared);

    // listener
public:
    class Listener {
    public:
        virtual void OnMessage(const uint8_t *message, uint64_t message_len) = 0;
        virtual void OnError(SystemError)                                    = 0;
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

    void onErr(SystemError);

protected:
    IO_Handle_p                   handler_;
    typename FAMILY::EndPointInfo local_endpoint_;
    typename FAMILY::EndPointInfo remote_endpoint_;
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
    void ChannelIn() override;
    void ChannelOut() override;
    void onChannelClose();
    void onChannelError(SystemError);
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
        virtual void onChannelDisConnect()     = 0;
        virtual void onReceive(ABuffer buffer) = 0;
        virtual void onSend(ABuffer buffer)    = 0;
        virtual void onError(SystemError)      = 0;

        virtual ~Listener() = default;
    };

    inline void SetListener(weak_ptr<Listener> listener) { this->listener_ = std::move(listener); }

protected:
    weak_ptr<Listener> listener_;

    void ChannelIn() override;
    void ChannelOut() override;
    void onChannelClose();
    void onChannelError(SystemError);
};


} // namespace wutils::network::channel


#endif // UTILS_CHANNEL_H