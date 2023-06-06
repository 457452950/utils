#include "wutils/network/Channel.h"
#include <cassert>
#include <iostream>
#include <utility>

#include "wutils/Debugger.hpp"
#include "wutils/logger/AsyncLogger.h"
#include "wutils/network/NetFactory.h"


namespace wutils::network {

using namespace debug;

static auto in_cb = [](socket_t sock, BaseChannel *data) {
    auto *ch = (ReadChannel *)data;
    ch->ChannelIn();
};
static auto out_cb = [](socket_t sock, BaseChannel *data) {
    auto *ch = (WriteChannel *)data;
    ch->ChannelOut();
};

void setCommonCallBack(ev_hdle_p handle) {
    handle->read_  = in_cb;
    handle->write_ = out_cb;
}


/********************************************
 * Timer
 *********************************************/
Timer::Timer(std::weak_ptr<ev_hdle_t> handle) {
    this->handler_             = std::make_unique<ev_hdler_t>();
    this->handler_->socket_    = CreateNewTimerfd();
    this->handler_->user_data_ = this;
    this->handler_->handle_    = std::move(handle);
    this->handler_->SetEvents(HandlerEventType::EV_IN);
    DEBUGADD("Timer");
}

Timer::~Timer() {
    DEBUGRM("Timer");
    this->Stop();
}

void Timer::ChannelIn() {
    uint64_t exp = 0;
    ::read(this->handler_->socket_, &exp, sizeof(exp));
    if(OnTime) {
        OnTime();
    }
}

bool Timer::Start(long time_value, long interval) {
    struct itimerspec next_time;

    next_time.it_value.tv_sec     = time_value / 1000L;
    next_time.it_value.tv_nsec    = (time_value % 1000L) * 1000'000L;
    next_time.it_interval.tv_sec  = interval / 1000L;
    next_time.it_interval.tv_nsec = (interval % 1000L) * 1000'000L;

    if(!SetTimerTime(this->handler_->socket_, SetTimeFlag::REL, &next_time)) {
        return false;
    }

    if(!this->active_) {
        this->handler_->Enable();
        this->active_ = true;
    }

    return true;
}

void Timer::Stop() {
    if(this->active_) {
        std::cout << "Timer::Stop() " << std::endl;
        this->handler_->DisEnable();
        this->active_ = false;
    }
}


/***********************************************************
 * AcceptorChannel
 ************************************************************/

AcceptorChannel::AcceptorChannel(std::weak_ptr<ev_hdle_t> handle) {
    this->handler_             = std::make_unique<ev_hdler_t>();
    this->handler_->handle_    = handle;
    this->handler_->user_data_ = this;
}

AcceptorChannel::~AcceptorChannel() {
    if(this->handler_) {
        if(this->handler_->IsEnable())
            this->handler_->DisEnable();
    }
}

bool wutils::network::AcceptorChannel::Start(const EndPointInfo &local_endpoint, bool shared) {
    this->local_endpoint_ = local_endpoint;

    auto socket = MakeListenedSocket(local_endpoint_, shared);
    if(socket == -1) {
        std::cout << "make listened socket err " << ErrorToString(GetError()) << std::endl;
        return false;
    }

    this->handler_->socket_ = socket;
    this->handler_->SetEvents(HandlerEventType::EV_IN);
    this->handler_->Enable();

    return true;
}

void AcceptorChannel::ChannelIn() {
    assert(this->handler_);

    EndPointInfo ei;
    auto         cli = wutils::network::Accept4(this->handler_->socket_, ei, SOCK_NONBLOCK);

    if(cli <= 0) { // error
        if(OnError) {
            OnError(ErrorToString(GetError()));
        }
    } else {
        if(!OnAccept) {
            return;
        }

        auto handler     = std::make_unique<ev_hdler_t>();
        handler->socket_ = cli;
        handler->handle_ = this->handler_->handle_;

        OnAccept(local_endpoint_, ei, std::move(handler));
    }
}

/***********************************************************
 * UDPPointer
 ************************************************************/

UDPPointer::UDPPointer(std::weak_ptr<ev_hdle_t> handle) {
    std::cout << "UDPPointer " << std::endl;

    this->handler_             = std::make_unique<ev_hdler_t>();
    this->handler_->handle_    = handle;
    this->handler_->user_data_ = this;
}

UDPPointer::~UDPPointer() {
    if(this->handler_) {
        if(this->handler_->IsEnable())
            this->handler_->DisEnable();
    }
}

bool wutils::network::UDPPointer::Start(const EndPointInfo &local_endpoint, bool shared) {
    this->local_endpoint_ = local_endpoint;

    auto socket = MakeBindedSocket(local_endpoint_, shared);
    if(socket == -1) {
        return false;
    }

    this->handler_->socket_ = socket;

    this->handler_->SetEvents(HandlerEventType::EV_IN);
    this->handler_->Enable();
    return true;
}

void UDPPointer::ChannelIn() {
    // std::cout << "accpet channel in" << std::endl;

    EndPointInfo ei;
    uint8_t      buf[MAX_UDP_BUFFER_LEN]{0};

    auto recv_len = RecvFrom(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, ei);

    if(recv_len <= 0) { // error
        onErr(GetError());
    } else {
        if(!OnMessage) {
            return;
        }

        OnMessage(local_endpoint_, ei, buf, recv_len);
    }
}

void UDPPointer::onErr(int err) {
    if(err != 0) {
        if(this->OnError) {
            this->OnError(ErrorToString(err));
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
        this->onErr(GetError());
        return false;
    }

    return true;
}


/***********************************************************
 * UDPChannel
 ************************************************************/

UDPChannel::UDPChannel(std::weak_ptr<ev_hdle_t> handle) {
    this->handler_             = std::make_unique<ev_hdler_t>();
    this->handler_->user_data_ = this;
    this->handler_->handle_    = handle;
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

    std::cout << "UDPChannel socket " << socket << std::endl;

    bool ok = ConnectToHost(socket, remote_endpoint_);
    // like not be fail
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
    // std::cout << "accpet channel in" << std::endl;

    EndPointInfo ei;
    uint8_t      buf[MAX_UDP_BUFFER_LEN]{0};

    // auto recv_len = RecvFrom(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, &ei);
    auto recv_len = recv(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, 0);

    // if(ei != this->remote_endpoint_) {
    //     std::cout << "not wanted remote " << std::endl;
    //     return;
    // }

    if(recv_len <= 0) { // error
        onErr(GetError());
    } else {
        if(listener_.expired()) {
            return;
        }

        listener_.lock()->OnMessage(buf, recv_len);
    }
}

void UDPChannel::onErr(int err) {
    if(err != 0) {
        if(!listener_.expired()) {
            listener_.lock()->OnError(ErrorToString(err));
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
        this->onErr(GetError());
        return false;
    }

    return true;
}


/***********************************************************
 * Channel
 ************************************************************/

Channel::Channel(const EndPointInfo &local, const EndPointInfo &remote, std::unique_ptr<ev_hdler_t> h) :
    local_endpoint_(local), remote_endpoint_(remote), event_handler_(std::move(h)) {

    assert(this->event_handler_);

    this->event_handler_->user_data_ = this;
    this->event_handler_->SetEvents(HandlerEventType::EV_IN);
    this->event_handler_->Enable();

    auto [ip, port] = EndPointInfo::Dump(this->remote_endpoint_);
    std::cout << "Channel " << ip << port << std::endl;
}

Channel::~Channel() {
    this->event_handler_->DisEnable();
    this->ShutDown(SHUT_RDWR);
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

    // has buf
    if(max_send_buf_size_ != 0 && !send_buf->IsEmpty() > 0) {
        // push data to buf
        auto l = this->send_buf->Write(send_message, message_len);
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
        auto eno = GetError();
        if(eno == EAGAIN || eno == EWOULDBLOCK) {
        } else {
            this->onChannelError(GetError());
        }
        return;
    }

    if(res < message_len) {
        if(max_send_buf_size_ == 0) {
            abort();
            return;
        }

        auto l = this->send_buf->Write(send_message + res, message_len - (uint32_t)res);
        if(l != (message_len - res)) {
            abort();
            return;
        }

        auto events = this->event_handler_->GetEvents();
        events |= (HandlerEventType::EV_OUT);
        this->event_handler_->SetEvents(events);
    } else {
        //        std::cout << "send all " << message_len << std::endl;
    }
}

void Channel::SetRecvBufferMaxSize(uint64_t max_size) {
    this->max_recv_buf_size_ = std::min<uint64_t>(max_size, MAX_CHANNEL_RECV_BUFFER_SIZE);

    recv_buf->Init(this->max_recv_buf_size_);
}

void Channel::SetSendBufferMaxSize(uint64_t max_size) {
    this->max_send_buf_size_ = std::min<uint64_t>(max_size, MAX_CHANNEL_SEND_BUFFER_SIZE);

    send_buf->Init(this->max_send_buf_size_);
}

void Channel::ChannelIn() {
    assert(this->event_handler_);
    assert(this->max_recv_buf_size_);
    assert(this->recv_buf->GetWriteableBytes() != 0);
    assert(!this->listener_.expired());

    auto r_buff = recv_buf;

    int64_t recv_len = 0;
    recv_len         = ::recv(this->event_handler_->socket_, r_buff->PeekWrite(), r_buff->GetWriteableBytes(), 0);
    if(recv_len == 0) { // has emitted in recv_buf->Write
        this->onChannelClose();
        return;
    } else if(recv_len == -1) {
        auto eno = GetError();
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

    //    while(!this->recv_buf->IsEmpty()) {
    //        this->listener_.lock()->onReceive(this->recv_buf->ConstPeekRead(), this->recv_buf->GetReadableBytes());
    //        this->recv_buf->SkipReadBytes(this->recv_buf->GetReadableBytes());
    //    }
}

// can write
void Channel::ChannelOut() {
    assert(this->event_handler_);

    // 无缓冲设计或无缓冲数据
    if(this->max_send_buf_size_ == 0 || this->send_buf->IsEmpty()) {
        auto events = this->event_handler_->GetEvents();
        events |= (~HandlerEventType::EV_OUT);
        this->event_handler_->SetEvents(events);
        return;
    }

    // 发送缓冲数据
    send_buf->ReadUntil([&](const uint8_t *data, int len) -> int64_t {
        auto send_len = ::send(this->event_handler_->socket_, data, len, 0);
        if(send_len == 0) {
            this->onChannelClose();
            return 0;
        }
        if(send_len < 0) {
            auto eno = GetError();
            if(eno == EAGAIN || eno == EWOULDBLOCK) {
                return 0;
            }
            this->onChannelError(GetError());
        }

        return send_len;
    });

    // 无缓冲数据时，停止
    if(this->send_buf->IsEmpty()) {
        auto events = this->event_handler_->GetEvents();
        events      = events | (~HandlerEventType::EV_OUT);
        this->event_handler_->SetEvents(events);
    }
}

void Channel::onChannelClose() {
    if(!this->listener_.expired())
        this->listener_.lock()->onChannelDisConnect();
}

void Channel::onChannelError(int error_code) {
    std::cout << "error " << ErrorToString(error_code) << std::endl;
    if(!this->listener_.expired())
        this->listener_.lock()->onError(ErrorToString(error_code));
}


} // namespace wutils::network
