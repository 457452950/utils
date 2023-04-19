#include "WNetWork/WChannel.h"
#include <cassert>
#include <iostream>

#include "WDebugger.hpp"

#include "WNetWork/WNetFactory.h"


namespace wlb::network {

using namespace debug;

WTimer::WTimer(event_handle_p handle) : handle_(handle) {
    this->timer_fd_ = CreateNewTimerfd();
    DEBUGADD("WTimer");
}
WTimer::~WTimer() {
    DEBUGRM("WTimer");
    this->Stop();
    if(this->timer_fd_) {
        ::close(this->timer_fd_);
        this->timer_fd_ = 0;
    }
}

void WTimer::ChannelIn() {
    uint64_t exp = 0;
    ::read(this->timer_fd_, &exp, sizeof(exp));
    if(OnTime) {
        OnTime();
    }
}

bool WTimer::Start(long time_value, long interval) {
    assert(!this->active_);
    assert(this->OnTime);

    struct itimerspec next_time {
        0
    };
    next_time.it_value.tv_sec     = time_value / 1000L;
    next_time.it_value.tv_nsec    = (time_value % 1000L) * 1000'000L;
    next_time.it_interval.tv_sec  = interval / 1000L;
    next_time.it_interval.tv_nsec = (interval % 1000L) * 1000'000L;
    if(!SetTimerTime(this->timer_fd_, SetTimeFlag::REL, &next_time)) {
        return false;
    }

    item_ = this->handle_->NewSocket(new event_handle_t::option_type{
            .socket_ = this->timer_fd_, .user_data_ = this, .events_ = KernelEventType::EV_IN});

    this->active_ = true;

    return true;
}

void WTimer::Stop() {
    if(this->active_) {
        this->handle_->DelSocket(item_);
        this->active_ = false;
    }
}


WAccepterChannel::WAccepterChannel(base_socket_type socket, const WEndPointInfo &endpoint, event_context_p context) {
    this->listen_socket_  = socket;
    this->local_endpoint_ = endpoint;
    this->event_context_  = context;

    assert(context->event_handle_);
    assert(this->listen_socket_ != -1);

    context->event_handle_->NewSocket(new event_handle_t::option_type{
            .socket_ = this->listen_socket_, .user_data_ = this, .events_ = KernelEventType::EV_IN});
}

WAccepterChannel::~WAccepterChannel() {
    if(this->listen_socket_) {
        ::close(this->listen_socket_);
        this->listen_socket_ = 0;
    }
    this->event_context_ = nullptr;
}

void WAccepterChannel::ChannelIn() {
    // std::cout << "accpet channel in" << std::endl;

    WEndPointInfo ei;
    auto          cli = wlb::network::Accept(this->listen_socket_, &ei);

    assert(event_context_);
    assert(event_context_->channel_factory_);
    assert(event_context_->session_factory_);

    if(cli <= 0) {
        event_context_->onAcceptError(errno);
    } else {
        // std::cout << "accepts not null and call " << std::endl;
        bool isAccept = event_context_->onAccept(cli, ei);
        if(isAccept) {
            // std::cout << "accept !!" << std::endl;
            auto newChannel = event_context_->channel_factory_->CreateChannel();
            newChannel->Init(cli, ei);
            newChannel->SetEventHandle(event_context_->event_handle_);

            // std::cout << "accept new session" << std::endl;
            event_context_->session_factory_->CreateSession(std::move(newChannel));

            // std::cout << "accept over" << std::endl;
        }
    }
}

WChannel::WChannel(uint16_t buffer_size) {
    DEBUGADD("WChannel");

    this->recv_buf_size_ = buffer_size;
    this->recv_buf_      = new uint8_t[this->recv_buf_size_]{};
    NEWADD;

    this->send_buf_size_ = buffer_size;
    this->send_buf_      = new uint8_t[this->send_buf_size_]{};
    NEWADD;
}

WChannel::~WChannel() {
    DEBUGRM("WChannel");
    // std::cout << "~WChannel" << std::endl;

    if(event_handle_ != nullptr) {
        // this->event_handle_->DelSocket(this->option_item_);
    }

    if(this->client_socket_) {
        // ::close(this->client_socket_);
        this->client_socket_ = 0;
    }
    if(this->recv_buf_) {
        DELADD;
        delete[] this->recv_buf_;
        this->recv_buf_ = nullptr;
    }
    if(this->send_buf_) {
        DELADD;
        delete[] this->send_buf_;
        this->send_buf_ = nullptr;
    }

    event_handle_ = nullptr;
}

void WChannel::Init(base_socket_type socket, const WEndPointInfo &remote_endpoint) {
    this->client_socket_   = socket;
    this->remote_endpoint_ = remote_endpoint;
}


void WChannel::ChannelIn() {
    assert(this->event_handle_);
    assert(this->recv_buf_size_);

    // std::cout << "WChannel ChannelIn this& " << this << std::endl;

    ssize_t recv_len = 0;
    recv_len         = ::recv(this->client_socket_, this->recv_buf_, this->recv_buf_size_, 0);
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

    assert(this->event_handle_);
    auto send_len = ::send(this->client_socket_, send_buf_, send_size_, 0);
    // std::cout << "WChannel::ChannelOut send message : " << (char *)send_buf_ << std::endl;
    // std::cout << "WChannel::ChannelOut send len: " << send_len << std::endl;

    if(send_len == -1) {
        if(this->listener_)
            this->onChannelError(errno);
    } else if(send_len == 0) {
        this->onChannelClose();
    } else {
        if(send_len == send_size_) {
            // send all, over
            (*option_item_)->events_ &= (~KernelEventType::EV_OUT);
            event_handle_->ModifySocket(option_item_);
        } else {
            ::memcpy(send_buf_, send_buf_ + send_len, send_size_ - send_len);
        }
        send_size_ -= send_len;
    }
}

void WChannel::SetEventHandle(event_handle_p handle) {
    this->event_handle_ = handle;

    if(this->event_handle_ != nullptr) {
        option_item_ = event_handle_->NewSocket(new event_handle_t::option_type{
                .socket_ = this->client_socket_, .user_data_ = this, .events_ = KernelEventType::EV_IN});
    } else {
        //
    }
}

void WChannel::Send(void *send_message, uint64_t message_len) {
    // std::cout << "wchannel send" << std::endl;
    if(this->send_size_ + message_len > send_buf_size_) {
        if(this->listener_)
            // TODO:错误码
            // over size
            this->onChannelError(-1);
        else {
            std::cout << "no write error callback " << std::endl;
        }
    }

    assert(this->send_buf_);

    ::memcpy(this->send_buf_, (uint8_t *)send_message, message_len);
    this->send_size_ += message_len;

    // std::cout << "send message : " << (char *)send_buf_ << std::endl;

    (*option_item_)->events_ |= KernelEventType::EV_OUT;
    // std::cout << "WChannel Send & " << (*option_item_)->user_data_ << std::endl;
    if(event_handle_ != nullptr) {
        event_handle_->ModifySocket(option_item_);
    }
}

void WChannel::CloseChannel() {
    // std::cout << "WChannel::CloseChannel()" << std::endl;
    ::shutdown(this->client_socket_, SHUT_RDWR);
}
void WChannel::onChannelClose() {
    // std::cout << "WChannel::onChannelClose()" << std::endl;
    // std::cout << "WChannel::onChannelClose() DelSocket" << std::endl;
    this->event_handle_->DelSocket(option_item_);
    close(this->client_socket_);
    this->listener_->onChannelDisConnect();
}
void WChannel::onChannelError(uint64_t error_code) {
    this->listener_->onError(error_code);
    std::cout << "error " << strerror(errno) << std::endl;
}


} // namespace wlb::network
