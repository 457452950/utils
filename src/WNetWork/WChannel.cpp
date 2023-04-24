#include "WNetWork/WChannel.h"
#include <cassert>
#include <iostream>

#include "WDebugger.hpp"

#include "WNetWork/WNetFactory.h"


namespace wlb::network {

using namespace debug;

/********************************************
 * Timer
 *********************************************/
WTimer::WTimer(event_handle_p handle) {
    this->handler_ = event_handler_t::CreateHandler(CreateNewTimerfd(), this, handle);
    this->handler_->SetEvents(HandlerEventType::EV_IN);
    DEBUGADD("WTimer");
}
WTimer::~WTimer() {
    DEBUGRM("WTimer");
    this->Stop();
    if(this->handler_) {
        delete this->handler_;
        this->handler_ = nullptr;
    }
}

void WTimer::ChannelIn() {
    uint64_t exp = 0;
    ::read(this->handler_->socket_, &exp, sizeof(exp));
    if(OnTime) {
        OnTime();
    }
}

bool WTimer::Start(long time_value, long interval) {
    assert(!this->active_);

    struct itimerspec next_time {
        0
    };
    next_time.it_value.tv_sec     = time_value / 1000L;
    next_time.it_value.tv_nsec    = (time_value % 1000L) * 1000'000L;
    next_time.it_interval.tv_sec  = interval / 1000L;
    next_time.it_interval.tv_nsec = (interval % 1000L) * 1000'000L;
    if(!SetTimerTime(this->handler_->socket_, SetTimeFlag::REL, &next_time)) {
        return false;
    }

    this->handler_->Enable();

    this->active_ = true;

    return true;
}

void WTimer::Stop() {
    if(this->active_) {
        this->handler_->DisEnable();
        this->active_ = false;
    }
}


/***********************************************************
 * WAccepterChannel
 ************************************************************/

WAccepterChannel::WAccepterChannel(const WEndPointInfo &endpoint, event_handle_p handle) {
    this->local_endpoint_ = endpoint;
    
    auto socket    = MakeListenedSocket(local_endpoint_);
    this->handler_ = event_handler_t::CreateHandler(socket, this, handle);
    this->handler_->SetEvents(HandlerEventType::EV_IN);
    this->handler_->Enable();
}

WAccepterChannel::~WAccepterChannel() {
    if(this->handler_) {
        if(this->handler_->IsEnable())
            this->handler_->DisEnable();

        delete this->handler_;
        this->handler_ = nullptr;
    }
}

void WAccepterChannel::ChannelIn() {
    // std::cout << "accpet channel in" << std::endl;

    WEndPointInfo ei;
    auto          cli = wlb::network::Accept4(this->handler_->socket_, &ei, SOCK_NONBLOCK);

    if(cli <= 0) { // error
        if(OnError) {
            OnError(ErrorToString(GetError()));
        }
    } else {
        if(!OnAccept) {
            return;
        }

        auto handler = event_handler_t::CreateHandler(cli, nullptr, this->handler_->handle_);

        auto channel = OnAccept(local_endpoint_, ei, handler);
        if(channel != nullptr) { // accept
            handler->user_data_ = channel;
        } else {
            // not accept
            ::close(cli);
        }
    }
}

/***********************************************************
 * WUDPChannel
 ************************************************************/

WUDPChannel::WUDPChannel(const WEndPointInfo &endpoint, event_handle_p handle) {
    auto socket = MakeBindedSocket(local_endpoint_);

    this->handler_ = event_handler_t::CreateHandler(socket, this, handle);
    this->handler_->SetEvents(HandlerEventType::EV_IN | HandlerEventType::EV_OUT);
    this->handler_->Enable();

    this->local_endpoint_ = endpoint;
}

WUDPChannel::~WUDPChannel() {
    if(this->handler_) {
        if(this->handler_->IsEnable())
            this->handler_->DisEnable();

        delete this->handler_;
        this->handler_ = nullptr;
    }
}

static constexpr int MAX_UDP_BUFFER_LEN = 1500;

void WUDPChannel::ChannelIn() {
    // std::cout << "accpet channel in" << std::endl;

    WEndPointInfo ei;
    uint8_t       buf[MAX_UDP_BUFFER_LEN];

    auto recv_len = wlb::network::RecvFrom(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, &ei);

    if(recv_len <= 0) { // error
        if(OnError) {
            OnError(ErrorToString(GetError()));
        }
    } else {
        if(!OnMessage) {
            return;
        }

        OnMessage(local_endpoint_, ei, buf, recv_len, this->handler_);
    }
}

void WUDPChannel::ChannelOut() {

}

/***********************************************************
 * WChannel
 ************************************************************/

WChannel::WChannel(WEndPointInfo local, WEndPointInfo remote, event_handler_p h) :
    local_endpoint_(local), remote_endpoint_(remote), event_handler_(h) {
    DEBUGADD("WChannel");

    assert(this->event_handler_);

    this->event_handler_->SetEvents(HandlerEventType::EV_IN | HandlerEventType::EV_OUT);
    this->event_handler_->Enable();
}

WChannel::~WChannel() {
    DEBUGRM("WChannel");
    // std::cout << "~WChannel" << std::endl;

    if(event_handler_ != nullptr) {
        delete event_handler_;
        event_handler_ = nullptr;
    }
    this->FreeRecvBuffer();
    this->FreeSendBuffer();
}

bool WChannel::Init() { return true; }

void WChannel::ShutDown(int how) {
    // std::cout << "WChannel::ShutDown()" << std::endl;

    assert(how >= SHUT_RD);
    assert(how <= SHUT_RDWR);

    ::shutdown(this->event_handler_->socket_, how);
}

void WChannel::CloseChannel() {}

void WChannel::Send(uint8_t *send_message, uint64_t message_len) {
    assert(message_len <= MAX_CHANNEL_SEND_SIZE);

    // has buf
    if (send_len_ > 0) {

    }

    // try to send
    // auto res = ::send()

}

void WChannel::SetRecvBufferMaxSize(uint64_t size) {
    assert(size <= MAX_CHANNEL_RECV_BUFFER_SIZE);

    if(this->recv_buf_size_ == size) {
        return;
    }

    // event state has changed
    // 0 -> newsize  or  oldsize -> 0
    // if(this->recv_buf_size_ * size == 0) {
    //     if(this->recv_buf_size_ > 0) { // 0 -> newsize
    //         this->event_handler_->SetEvents(this->event_handler_->GetEvents() | HandlerEventType::EV_IN);
    //         this->event_handler_->Enable();
    //     } else { // oldsize -> 0
    //         auto events = this->event_handler_->GetEvents();
    //         events &= (!HandlerEventType::EV_IN);

    //         // neither EV_IN and EV_OUT
    //         this->event_handler_->SetEvents(this->event_handler_->GetEvents() | HandlerEventType::EV_IN);
    //         if(events = 0) {
    //             this->event_handler_->DisEnable();
    //         }
    //     }
    // }

    this->FreeSendBuffer();
    this->recv_buf_size_ = size;
    if(this->recv_buf_size_ > 0) {
        this->recv_buf_ = new uint8_t[this->recv_buf_size_]{0};
    }
}
void WChannel::SetSendBufferMaxSize(uint64_t size) {
    assert(size <= MAX_CHANNEL_SEND_BUFFER_SIZE);

    if(this->send_buf_size_ == size) {
        return;
    }

    this->FreeSendBuffer();
    this->send_buf_size_ = size;
    if(this->send_buf_size_ > 0) {
        this->send_buf_ = new uint8_t[this->send_buf_size_]{0};
    }
}

void WChannel::FreeRecvBuffer() {
    if(this->recv_buf_) {
        DELADD;
        delete[] this->recv_buf_;
        this->recv_buf_ = nullptr;
    }
}

void WChannel::FreeSendBuffer() {
    if(this->send_buf_) {
        DELADD;
        delete[] this->send_buf_;
        this->send_buf_ = nullptr;
    }
}

void WChannel::ChannelIn() {
    assert(this->event_handler_);
    assert(this->recv_buf_size_);

    // std::cout << "WChannel ChannelIn this& " << this << std::endl;

    ssize_t recv_len = 0;
    // recv_len         = ::recv(this->client_socket_, this->recv_buf_, this->recv_buf_size_, 0);
    // std::cout << "WChannel::ChannelIn() channel in recv " << recv_len << std::endl;

    if(this->listener_ == nullptr) {
        // std::cout << "WChannel::ChannelIn() listener is nullptr." << std::endl;
        return;
    }


    if(recv_len < 0) {
        std::cout << "recv len -1 :" << strerror(errno) << std::endl;
        this->onChannelError(errno);
    } else if(recv_len == 0) {
        this->onChannelClose();
    } else {
        this->listener_->onReceive(this->recv_buf_, recv_len);
        // std::cout << "WChannel in on read end " << std::endl;
    }
    // std::cout << "WChannel in end " << std::endl;
}

void WChannel::ChannelOut() {
    // std::cout << "WChannel::ChannelOut " << std::endl;

    assert(this->event_handler_);
    // auto send_len = ::send(this->client_socket_, send_buf_, send_size_, 0);
    // std::cout << "WChannel::ChannelOut send message : " << (char *)send_buf_ << std::endl;
    // std::cout << "WChannel::ChannelOut send len: " << send_len << std::endl;

    // if(send_len == -1) {
    //     if(this->listener_)
    //         this->onChannelError(errno);
    // } else if(send_len == 0) {
    //     this->onChannelClose();
    // } else {
    //     if(send_len == send_size_) {
    //         // send all, over
    //         (*option_item_)->events_ &= (~HandlerEventType::EV_OUT);
    //         event_handle_->ModifySocket(option_item_);
    //     } else {
    //         ::memcpy(send_buf_, send_buf_ + send_len, send_size_ - send_len);
    //     }
    //     send_size_ -= send_len;
    // }
}

void WChannel::onChannelClose() {
    // std::cout << "WChannel::onChannelClose()" << std::endl;
    // std::cout << "WChannel::onChannelClose() DelSocket" << std::endl;
//     this->event_handle_->DelSocket(option_item_);
//     close(this->client_socket_);
    this->listener_->onChannelDisConnect();
}
void WChannel::onChannelError(uint64_t error_code) {
    // this->listener_->onError(error_code);
    std::cout << "error " << strerror(errno) << std::endl;
}


} // namespace wlb::network
