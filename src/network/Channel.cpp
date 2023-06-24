#include "wutils/network/Channel.h"
#include <cassert>
#include <iostream>
#include <utility>

#include "wutils/network/NetFactory.h"

namespace wutils::network::channel {

static auto in_cb = [](socket_t sock, BaseChannel *data) {
    auto *ch = (ReadChannel *)data;
    ch->ChannelIn();
};
static auto out_cb = [](socket_t sock, BaseChannel *data) {
    auto *ch = (WriteChannel *)data;
    ch->ChannelOut();
};

void setCommonCallBack(IO_Context *handle) {
    handle->read_  = in_cb;
    handle->write_ = out_cb;
}

/********************************************
 * Timer
 *********************************************/
Timer::Timer(std::weak_ptr<IO_Context> handle) {
    this->handler_ = make_shared<IO_Handle_t>();

    this->handler_->socket_    = timer_socket_.GetNativeSocket();
    this->handler_->user_data_ = this;
    this->handler_->handle_    = std::move(handle);
    this->handler_->SetEvents(event::EventType::EV_IN);
}

Timer::~Timer() { this->Stop(); }

void Timer::ChannelIn() {
    std::cout << " timer channelin" << std::endl;

    this->timer_socket_.Read();

    if(OnTime) {
        OnTime();
    }
}

template <typename Rep, typename Period>
bool Timer::Once(const std::chrono::duration<Rep, Period> &rtime) {
    this->timer_socket_.SetOnce(rtime);

    if(!this->handler_->IsEnable()) {
        this->handler_->Enable();
    }

    return true;
}
template <typename Rep, typename Period>
bool Timer::Repeat(const std::chrono::duration<Rep, Period> &rtime) {
    this->timer_socket_.SetRepeat(rtime);

    if(!this->handler_->IsEnable()) {
        this->handler_->Enable();
    }

    return true;
}

void Timer::Stop() {
    if(this->handler_->IsEnable()) {
        std::cout << "Timer::Stop() " << std::endl;
        this->handler_->DisEnable();
    }
}


/***********************************************************
 * AcceptorChannel
 ************************************************************/

/***********************************************************
 * UDPPointer
 ************************************************************/


void UDPPointer::ChannelIn() {
    EndPointInfo ei;
    uint8_t      buf[MAX_UDP_BUFFER_LEN]{0};

    auto recv_len = RecvFrom(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, ei);

    if(recv_len <= 0) { // error
        onErr(SystemError::GetSysErrCode());
    } else {
        if(!OnMessage) {
            return;
        }

        OnMessage(local_endpoint_, ei, buf, recv_len);
    }
}

void UDPPointer::onErr(SystemError err) {
    if(err.Code() != 0) {
        if(this->OnError) {
            this->OnError(err);
        }
    }
}

bool UDPPointer::SendTo(const uint8_t *send_message, uint32_t message_len, const EndPointInfo &remote) {
    assert(this->handler_->IsEnable());
    assert(message_len <= MAX_WAN_UDP_PACKAGE_LEN);

    auto [ip, port] = EndPointInfo::Dump(remote);
    std::cout << "UDPPointer::SendTo " << ip << " : " << port << " " << this->handler_->socket_ << " "
              << " msg:" << send_message << " size " << (size_t)message_len << std::endl;

    auto len = ::sendto(
            this->handler_->socket_, (void *)send_message, message_len, 0, remote.GetAddr(), remote.GetSockSize());
    if(len == -1) {
        std::cout << "send to err " << std::endl;
        this->onErr(SystemError::GetSysErrCode());
        return false;
    }

    return true;
}


/***********************************************************
 * UDPChannel
 ************************************************************/

UDPChannel::UDPChannel(std::weak_ptr<ev_hdle_t> handle) {
    this->handler_             = make_shared<ev_hdler_t>();
    this->handler_->user_data_ = this;
    this->handler_->handle_    = std::move(handle);
}

UDPChannel::~UDPChannel() {
    if(this->handler_) {
        if(this->handler_->IsEnable())
            this->handler_->DisEnable();
    }
}

bool UDPChannel::Start(const EndPointInfo &local_ep, const EndPointInfo &remote_ep, bool shared) {

    this->local_endpoint_  = local_ep;
    this->remote_endpoint_ = remote_ep;

    auto socket = MakeBindedSocket(local_endpoint_, shared);
    if(socket == -1) {
        return false;
    }


    bool ok = ConnectToHost(socket, remote_endpoint_);
    // may not be fail
    // if(!ok) {
    //     assert("UDPChannel ConnectToHost err ");
    //     std::cout << "UDPChannel ConnectToHost " << socket << std::endl;
    // }

    this->handler_->socket_ = socket;
    this->handler_->SetEvents(HandlerEventType::EV_IN);
    this->handler_->Enable();

    return true;
}

void UDPChannel::ChannelIn() {
    std::cout << "UDPChannel channel in" << std::endl;

    EndPointInfo ei;
    uint8_t      buf[MAX_UDP_BUFFER_LEN]{0};

    // auto recv_len = RecvFrom(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, &ei);
    auto recv_len = recv(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, 0);

    // if(ei != this->remote_endpoint_) {
    //     std::cout << "not wanted remote " << std::endl;
    //     return;
    // }

    if(recv_len <= 0) { // error
        onErr(SystemError::GetSysErrCode());
    } else {
        if(listener_.expired()) {
            return;
        }

        listener_.lock()->OnMessage(buf, recv_len);
    }
}

void UDPChannel::onErr(SystemError err) {
    if(err.Code() != 0) {
        if(!listener_.expired()) {
            listener_.lock()->OnError(err);
        }
    }
}

bool UDPChannel::Send(const uint8_t *send_message, uint32_t message_len) {
    assert(message_len <= MAX_WAN_UDP_PACKAGE_LEN);

    // auto len = ::sendto(this->handler_->socket_,
    //                     (void *)send_message,
    //                     message_len,
    //                     0,
    //                     remote_endpoint_.GetAddr(),
    //                     remote_endpoint_.GetSockSize());
    auto len = send(this->handler_->socket_, send_message, message_len, 0);

    if(len == -1) {
        std::cout << "send to err " << std::endl;
        this->onErr(SystemError::GetSysErrCode());
        return false;
    }

    return true;
}


/***********************************************************
 * Channel
 ************************************************************/

Channel::Channel(const EndPointInfo &local, const EndPointInfo &remote, ev_hdler_p h) :
    local_endpoint_(local), remote_endpoint_(remote), event_handler_(std::move(h)) {

    assert(this->event_handler_);

    this->event_handler_->user_data_ = this;
    this->event_handler_->SetEvents(HandlerEventType::EV_IN);
    this->event_handler_->Enable();

    auto [ip, port] = EndPointInfo::Dump(this->remote_endpoint_);
    std::cout << "Channel " << ip << port << std::endl;
}

Channel::~Channel() {
    this->ShutDown(SHUT_RDWR);

    if(this->event_handler_) {
        if(this->event_handler_->IsEnable())
            this->event_handler_->DisEnable();
    }
}

bool Channel::Init() { return true; }

void Channel::ShutDown(int how) {
    // std::cout << "Channel::ShutDown()" << std::endl;

    assert(how >= SHUT_RD);
    assert(how <= SHUT_RDWR);

    ::shutdown(this->event_handler_->socket_, how);
}

void Channel::Send(const uint8_t *send_message, uint32_t message_len) {
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
        events |= (HandlerEventType::EV_OUT);
        this->event_handler_->SetEvents(events);
        return;
    }

    // try to send
    auto res = ::send(this->event_handler_->socket_, send_message, message_len, 0);
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
        events |= (HandlerEventType::EV_OUT);
        this->event_handler_->SetEvents(events);
    } else {
        //        std::cout << "send all " << message_len << std::endl;
    }
}

void Channel::SetRecvBufferMaxSize(uint64_t max_size) {
    this->max_recv_buf_size_ = std::min<uint64_t>(max_size, MAX_CHANNEL_RECV_BUFFER_SIZE);

    recv_buf_->Init(this->max_recv_buf_size_);
}

void Channel::SetSendBufferMaxSize(uint64_t max_size) {
    this->max_send_buf_size_ = std::min<uint64_t>(max_size, MAX_CHANNEL_SEND_BUFFER_SIZE);

    send_buf_->Init(this->max_send_buf_size_);
}

void Channel::ChannelIn() {
    //    std::cout << "Channel channel in" << std::endl;

    assert(this->event_handler_);
    assert(this->max_recv_buf_size_);
    assert(this->recv_buf_->GetWriteableBytes() != 0);
    assert(!this->listener_.expired());

    auto r_buff = recv_buf_;

    int64_t recv_len = ::recv(this->event_handler_->socket_, r_buff->PeekWrite(), r_buff->GetWriteableBytes(), 0);

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
    //        this->listener_.lock()->onReceive(this->recv_buf_->ConstPeekRead(), this->recv_buf_->GetReadableBytes());
    //        this->recv_buf_->SkipReadBytes(this->recv_buf_->GetReadableBytes());
    //    }
}

// can write
void Channel::ChannelOut() {
    std::cout << "Channel channel out" << std::endl;
    assert(this->event_handler_);

    auto s_buf = this->send_buf_;

    // 无缓冲设计或无缓冲数据
    if(this->max_send_buf_size_ == 0 || s_buf->IsEmpty()) {
        auto events = this->event_handler_->GetEvents();
        events &= (~HandlerEventType::EV_OUT);
        this->event_handler_->SetEvents(events);
        return;
    }

    // 发送缓冲数据
    while(!s_buf->IsEmpty()) {
        auto send_len = ::send(this->event_handler_->socket_, s_buf->PeekRead(), s_buf->GetReadableBytes(), 0);
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
        events      = events & (~HandlerEventType::EV_OUT);
        this->event_handler_->SetEvents(events);
    }
}

void Channel::onChannelClose() {
    if(!this->listener_.expired())
        this->listener_.lock()->onChannelDisConnect();
}

void Channel::onChannelError(SystemError error) {
    std::cout << "error " << error << std::endl;
    if(!this->listener_.expired())
        this->listener_.lock()->onError(error);
}


/***********************************************************
 * ASChannel
 ************************************************************/

ASChannel::ASChannel(const EndPointInfo &local, const EndPointInfo &remote, ev_hdler_p h) :
    local_endpoint_(local), remote_endpoint_(remote), event_handler_(std::move(h)) {

    assert(this->event_handler_);

    this->event_handler_->user_data_ = this;

    auto [ip, port] = EndPointInfo::Dump(this->remote_endpoint_);
    std::cout << "ASChannel " << ip << port << std::endl;
}
ASChannel::~ASChannel() {
    this->ShutDown(SHUT_RDWR);

    if(this->event_handler_) {
        if(this->event_handler_->IsEnable())
            this->event_handler_->DisEnable();
    }
}
void ASChannel::ShutDown(int how) {
    // std::cout << "Channel::ShutDown()" << std::endl;

    assert(how >= SHUT_RD);
    assert(how <= SHUT_RDWR);

    ::shutdown(this->event_handler_->socket_, how);
}
void ASChannel::ARecv(ASChannel::ABuffer buffer) {
    //    std::cout << "ASChannel ARecv " << std::endl;
    assert(this->event_handler_);

    recv_buffer_ = buffer;

    this->event_handler_->SetEvents(this->event_handler_->GetEvents() | HandlerEventType::EV_IN);
    if(!this->event_handler_->IsEnable())
        this->event_handler_->Enable();
}
void ASChannel::ASend(ASChannel::ABuffer buffer) {
    //    std::cout << "ASChannel ASend " << std::endl;
    assert(this->event_handler_);

    send_buffer_ = buffer;

    this->event_handler_->SetEvents(this->event_handler_->GetEvents() | HandlerEventType::EV_OUT);
    if(!this->event_handler_->IsEnable())
        this->event_handler_->Enable();
}

void ASChannel::ChannelIn() {
    //    std::cout << "ASChannel Channel in" << std::endl;
    auto len = ::recv(this->event_handler_->socket_, recv_buffer_.buffer, recv_buffer_.buf_len, 0);

    if(len == 0 & recv_buffer_.buf_len != 0) {
        this->onChannelClose();
        return;
    }
    if(len < 0) {
        auto eno = SystemError::GetSysErrCode();
        if(eno == ECONNRESET) {
            this->onChannelClose();
            return;
        }
        this->onChannelError(eno);
        return;
    }

    this->event_handler_->SetEvents(this->event_handler_->GetEvents() & (~HandlerEventType::EV_IN));

    recv_buffer_.buf_len = len;

    if(!this->listener_.expired()) {
        this->listener_.lock()->onReceive(recv_buffer_);
    }
}
void ASChannel::ChannelOut() {
    auto len = ::send(this->event_handler_->socket_, send_buffer_.buffer, send_buffer_.buf_len, 0);
    if(len == 0 && send_buffer_.buf_len != 0) {
        this->onChannelClose();
        return;
    }
    if(len < 0) {
        auto eno = SystemError::GetSysErrCode();
        if(eno == ECONNRESET) {
            this->onChannelClose();
            return;
        }
        this->onChannelError(eno);
        return;
    }

    if(len == this->send_buffer_.buf_len) {
        this->event_handler_->SetEvents(this->event_handler_->GetEvents() & (~HandlerEventType::EV_OUT));

        if(!this->listener_.expired()) {
            this->listener_.lock()->onSend(send_buffer_);
        }
    } else {
        send_buffer_.buffer = send_buffer_.buffer + len;
        send_buffer_.buf_len -= len;
    }
}
void ASChannel::onChannelClose() {
    if(!this->listener_.expired())
        this->listener_.lock()->onChannelDisConnect();
}
void ASChannel::onChannelError(SystemError error) {
    std::cout << "error " << error << std::endl;
    if(!this->listener_.expired())
        this->listener_.lock()->onError(error);
}


} // namespace wutils::network::channel
