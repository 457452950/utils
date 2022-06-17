#include "WNetWork/WChannel.h"
#include <cassert>
#include <iostream>


namespace wlb::network {

WTimer::WTimer(WEventHandle *handle) : handle_(handle) { this->timer_fd_ = CreateNewTimerfd(); }
WTimer::~WTimer() {
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

    item_ = this->handle_->NewSocket(this->timer_fd_, KernelEventType::EV_IN, this);

    this->active_ = true;

    return true;
}

void WTimer::Stop() {
    this->handle_->DelSocket(item_);
    this->active_ = false;
}


WAccepterChannel::WAccepterChannel(base_socket_type socket, const WEndPointInfo &endpoint, event_context_p context) {
    this->listen_socket_  = socket;
    this->local_endpoint_ = endpoint;
    this->event_context_  = context;

    assert(context->event_handle_);
    assert(this->listen_socket_ != -1);

    context->event_handle_->NewSocket(this->listen_socket_, KernelEventType::EV_IN, this);
}

WAccepterChannel::~WAccepterChannel() {
    if(this->listen_socket_) {
        ::close(this->listen_socket_);
        this->listen_socket_ = 0;
    }
    this->event_context_ = nullptr;
}

void WAccepterChannel::ChannelIn() {
    std::cout << "accpet channel in" << std::endl;

    WEndPointInfo ei;
    auto          cli = wlb::network::Accept(this->listen_socket_, &ei, local_endpoint_.isv4);

    assert(event_context_);

    if(cli <= 0) {
        event_context_->onAcceptError(errno);
    } else {
        std::cout << "accpets not null and call " << std::endl;
        if(event_context_->onAccept(cli, ei)) {
            new WChannel(cli, ei, event_context_);
        }
    }
}

WChannel::WChannel(base_socket_type socket, WEndPointInfo &remote_endpoint, event_context_p context) {
    this->client_socket_   = socket;
    this->remote_endpoint_ = remote_endpoint;
    this->event_context_   = context;

    std::cout << "new wchannle " << socket << std::endl;

    std::cout << "WChannel WChannel this& " << this << std::endl;
    item_ = context->event_handle_->NewSocket(this->client_socket_, KernelEventType::EV_IN, this);

    this->recv_buf_size_ = event_context_->max_read_size_;
    this->recv_buf_      = new uint8_t[this->recv_buf_size_];

    this->send_buf_size_ = event_context_->max_read_size_;
    this->send_buf_      = new uint8_t[this->send_buf_size_];

    std::cout << "WChannel WChannel this& " << this << std::endl;
}

WChannel::~WChannel() {
    std::cout << "WChannel::~WChannel" << std::endl;
    this->event_context_->event_handle_->DelSocket(this->item_);

    if(this->client_socket_) {
        ::close(this->client_socket_);
    }
    if(this->recv_buf_) {
        delete[] this->recv_buf_;
        this->recv_buf_ = nullptr;
    }
    if(this->send_buf_) {
        delete[] this->send_buf_;
        this->send_buf_ = nullptr;
    }

    event_context_ = nullptr;
}

void WChannel::ChannelIn() {
    assert(this->event_context_);
    assert(this->recv_buf_size_);

    std::cout << "WChannel ChannelIn this& " << this << std::endl;

    ssize_t recv_len = 0;
    recv_len         = ::recv(this->client_socket_, this->recv_buf_, this->recv_buf_size_, 0);
    std::cout << "channel in recv " << recv_len << std::endl;

    if(recv_len == -1) {
        std::cout << "recv len -1 :" << strerror(errno) << std::endl;
        if(this->event_context_->onReadError) {
            this->event_context_->onReadError(this, errno);
        }
    } else {
        this->event_context_->onRead(this, this->recv_buf_, recv_len);
    }
}

void WChannel::ChannelOut() {
    std::cout << "WChannel::ChannelOut " << std::endl;

    assert(this->event_context_);
    auto send_len = ::send(this->client_socket_, send_buf_, send_size_, 0);
    // std::cout << "send message : " << (char *)send_buf_ << std::endl;
    std::cout << "WChannel::ChannelOut send len: " << send_len << std::endl;

    if(send_len == -1) {
        if(event_context_->onWriteError)
            event_context_->onWriteError(this, errno);
        std::cout << "error " << strerror(errno) << std::endl;
        // exit(-1);
    } else {
        if(send_len == send_size_) {
            // send all, over
            (*item_)->events_ &= (~KernelEventType::EV_OUT);
            event_context_->event_handle_->ModifySocket(item_);
        } else {
            ::memcpy(send_buf_, send_buf_ + send_len, send_size_ - send_len);
            send_size_ -= send_len;
        }
    }
}

void WChannel::Send(void *send_message, uint64_t message_len) {
    std::cout << "wchannel send" << std::endl;
    if(this->send_size_ + message_len > send_buf_size_) {
        if(this->event_context_->onWriteError)
            this->event_context_->onWriteError(this, 0);
        else
            std::cout << "no write error callback " << std::endl;
    }

    assert(this->send_buf_);

    ::memcpy(this->send_buf_, (uint8_t *)send_message, message_len);
    this->send_size_ += message_len;

    // std::cout << "send message : " << (char *)send_buf_ << std::endl;

    (*item_)->events_ |= KernelEventType::EV_OUT;
    std::cout << "WChannel Send & " << (*item_)->user_data_ << std::endl;
    event_context_->event_handle_->ModifySocket(item_);
}


} // namespace wlb::network
