#pragma once
#ifndef UTIL_TCPCONNECTION_H
#define UTIL_TCPCONNECTION_H

#include "IO_Context.h"
#include "NetWork.h"
#include "TCP.h"

namespace wutils::network::event {

template <class FAMILY>
class Channel : public IOEvent {
public:
    using Family       = FAMILY;
    using EndPointInfo = typename Family::EndPointInfo;
    using Socket       = ip::tcp::Socket<Family>;

    explicit Channel(const EndPointInfo &local, const EndPointInfo &remote, IO_Context::IO_Handle_p h) :
        local_endpoint_(local), remote_endpoint_(remote), event_handler_(std::move(h)) {

        assert(this->event_handler_);

        this->event_handler_->listener = this;
        this->event_handler_->SetEvents(event::EventType::EV_IN);
        this->event_handler_->Enable();

        this->socket_ = this->event_handler_->socket_;
    }
    ~Channel() override {
        this->ShutDown(SHUT_RDWR);

        if(this->event_handler_) {
            if(this->event_handler_->IsEnable())
                this->event_handler_->DisEnable();
        }
        this->socket_.Close();
    }

    bool Init() { return true; }
    void ShutDown(int how) {
        assert(how >= SHUT_RD);
        assert(how <= SHUT_RDWR);

        this->socket_.ShutDown(how);
    }

    virtual void Send(const uint8_t *send_message, uint32_t message_len) {
        assert(message_len <= MAX_CHANNEL_SEND_SIZE);

        auto s_buf = this->send_buf_;

        // has buf
        if(max_send_buf_size_ != 0 && !s_buf->IsEmpty() > 0) {
            // push data to buf
            auto l = s_buf->Write(send_message, message_len);
            if(l != message_len) {
                abort();
                return;
            }

            std::cout << "save all " << message_len << std::endl;

            auto events = this->event_handler_->GetEvents();
            events |= (event::EventType::EV_OUT);
            this->event_handler_->SetEvents(events);
            return;
        }

        // try to send
        auto res = this->socket_.Send(send_message, message_len);
        if(res < 0) {
            auto err = SystemError::GetSysErrCode();
            if(err == EAGAIN || err == EWOULDBLOCK) {
            } else {
                this->onChannelError(err);
            }
            return;
        }

        if(res < message_len) {
            assert(max_send_buf_size_ != 0);

            auto l = s_buf->Write(send_message + res, message_len - (uint32_t)res);
            assert(l == (message_len - res));

            auto events = this->event_handler_->GetEvents();
            events |= (event::EventType::EV_OUT);
            this->event_handler_->SetEvents(events);
        } else {
            //        std::cout << "send all " << message_len << std::endl;
        }
    }

    const EndPointInfo &GetLocalInfo() { return local_endpoint_; }
    const EndPointInfo &GetRemoteInfo() { return remote_endpoint_; }

protected:
    EndPointInfo            local_endpoint_;
    EndPointInfo            remote_endpoint_;
    Socket                  socket_;
    IO_Context::IO_Handle_p event_handler_;

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
    void     SetRecvBufferMaxSize(uint64_t max_size) {
        this->max_recv_buf_size_ = std::min<uint64_t>(max_size, MAX_CHANNEL_RECV_BUFFER_SIZE);

        recv_buf_->Init(this->max_recv_buf_size_);
    }
    void SetSendBufferMaxSize(uint64_t max_size) {
        this->max_send_buf_size_ = std::min<uint64_t>(max_size, MAX_CHANNEL_SEND_BUFFER_SIZE);

        send_buf_->Init(this->max_send_buf_size_);
    }

private:
    // receive buffer
    shared_ptr<RingBuffer> recv_buf_{new RingBuffer()};
    uint64_t               max_recv_buf_size_{0};
    // send buffer
    shared_ptr<RingBuffer> send_buf_{new RingBuffer()};
    uint64_t               max_send_buf_size_{0};

protected:
    // can override
    void EventIn() override {
        //    std::cout << "Channel channel in" << std::endl;

        assert(this->event_handler_);
        assert(this->max_recv_buf_size_);
        assert(this->recv_buf_->GetWriteableBytes() != 0);
        assert(!this->listener_.expired());

        auto r_buff = recv_buf_;

        int64_t recv_len = this->socket_.Recv(r_buff->PeekWrite(), r_buff->GetWriteableBytes());

        //    std::cout << "Channel channel in recv " << recv_len << std::endl;

        if(recv_len == 0) { // has emitted in recv_buf_->Write
            this->onChannelClose();
            return;
        } else if(recv_len == -1) {
            auto eno = SystemError::GetSysErrCode();
            if(eno == ECONNRESET) {
                this->onChannelClose();
                return;
            }
            if(eno == EAGAIN || eno == EWOULDBLOCK) {

            } else {
                this->onChannelError(eno);
            }
            return;
        }

        r_buff->UpdateWriteBytes(recv_len);

        r_buff->ReadUntil([&](const uint8_t *msg, uint32_t len) -> int64_t {
            this->listener_.lock()->onReceive(msg, len);
            return len;
        });

        //    while(!this->recv_buf_->IsEmpty()) {
        //        this->listener_.lock()->onReceive(this->recv_buf_->ConstPeekRead(),
        //        this->recv_buf_->GetReadableBytes());
        //        this->recv_buf_->SkipReadBytes(this->recv_buf_->GetReadableBytes());
        //    }
    }
    void EventOut() override {
        std::cout << "Channel channel out" << std::endl;
        assert(this->event_handler_);

        auto s_buf = this->send_buf_;

        // 无缓冲设计或无缓冲数据
        if(this->max_send_buf_size_ == 0 || s_buf->IsEmpty()) {
            auto events = this->event_handler_->GetEvents();
            events &= (~event::EventType::EV_OUT);
            this->event_handler_->SetEvents(events);
            return;
        }

        // 发送缓冲数据
        while(!s_buf->IsEmpty()) {
            auto send_len = this->socket_.Send(s_buf->PeekRead(), s_buf->GetReadableBytes());
            if(send_len == 0) {
                this->onChannelClose();
                return;
            }
            if(send_len < 0) {
                auto eno = SystemError::GetSysErrCode();
                if(eno == EAGAIN || eno == EWOULDBLOCK) {
                    break;
                }
                if(eno == ECONNRESET) {
                    this->onChannelClose();
                } else {
                    this->onChannelError(eno);
                }
                return;
            }

            s_buf->SkipReadBytes(send_len);
        }

        // 无缓冲数据时，停止监听 out 事件
        if(s_buf->IsEmpty()) {
            auto events = this->event_handler_->GetEvents();
            events      = events & (~event::EventType::EV_OUT);
            this->event_handler_->SetEvents(events);
        }
    }
    void onChannelClose() {
        if(!this->listener_.expired())
            this->listener_.lock()->onChannelDisConnect();
        this->socket_.Close();
    }
    void onChannelError(SystemError error) {
        std::cout << "error " << error << std::endl;
        if(!this->listener_.expired())
            this->listener_.lock()->onError(error);
        this->socket_.Close();
    }
};

};     // namespace wutils::network::event

#endif // UTIL_TCPCONNECTION_H
