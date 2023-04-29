#include "WNetWork/WChannel.h"
#include <cassert>
#include <iostream>

#include "AsyncLogger.h"
#include "WDebugger.hpp"
#include "WNetWork/WNetFactory.h"


namespace wlb::network {

using namespace debug;

/********************************************
 * Timer
 *********************************************/
WTimer::WTimer(std::weak_ptr<ev_hdle_t> handle) {
    this->handler_ = ev_hdler_t::CreateHandler(CreateNewTimerfd(), this, handle);
    this->handler_->SetEvents(HandlerEventType::EV_IN);
    DEBUGADD("WTimer");
}

WTimer::~WTimer() {
    DEBUGRM("WTimer");
    this->Stop();
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
        std::cout << "WTimer::Stop() " << std::endl;
        this->handler_->DisEnable();
        this->active_ = false;
    }
}


/***********************************************************
 * WAccepterChannel
 ************************************************************/

WAccepterChannel::WAccepterChannel(const WEndPointInfo &endpoint, std::weak_ptr<ev_hdle_t> handle) {
    this->local_endpoint_ = endpoint;

    auto socket = MakeListenedSocket(local_endpoint_);
    if(socket == -1) {
        std::cout << "make listened socket err " << ErrorToString(GetError()) << std::endl;
        assert(socket != -1);
    }

    this->handler_ = ev_hdler_t::CreateHandler(socket, this, handle);
    this->handler_->SetEvents(HandlerEventType::EV_IN);
    this->handler_->Enable();
}

WAccepterChannel::~WAccepterChannel() {
    if(this->handler_) {
        if(this->handler_->IsEnable())
            this->handler_->DisEnable();
    }
}

void WAccepterChannel::ChannelIn() {
    // std::cout << "accpet channel in" << std::endl;
    assert(this->handler_);

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

        auto handler = ev_hdler_t::CreateHandler(cli, nullptr, this->handler_->handle_);

        auto channel = OnAccept(local_endpoint_, ei, std::move(handler));
        if(channel != nullptr) { // accept
            handler->user_data_ = channel;
        } else {
            // not accept
            ::close(cli);
        }
    }
}

/***********************************************************
 * WUDP
 ************************************************************/

WUDP::WUDP(const WEndPointInfo &local_ep, std::weak_ptr<ev_hdle_t> handle) {
    this->local_endpoint_ = local_ep;

    auto socket = MakeBindedSocket(local_endpoint_);
    assert(socket != -1);

    std::cout << "WUDP socket " << socket << std::endl;

    this->handler_ = ev_hdler_t::CreateHandler(socket, this, handle);
    this->handler_->SetEvents(HandlerEventType::EV_IN);
    this->handler_->Enable();
}

WUDP::~WUDP() {
    if(this->handler_) {
        if(this->handler_->IsEnable())
            this->handler_->DisEnable();
    }
}

void WUDP::ChannelIn() {
    // std::cout << "accpet channel in" << std::endl;

    WEndPointInfo ei;
    uint8_t       buf[MAX_UDP_BUFFER_LEN]{0};

    auto recv_len = RecvFrom(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, &ei);

    if(recv_len <= 0) { // error
        onErr(GetError());
    } else {
        if(!OnMessage) {
            return;
        }

        OnMessage(local_endpoint_, ei, buf, recv_len);
    }
}

void WUDP::onErr(int err) {
    if(err != 0) {
        if(this->OnError) {
            this->OnError(ErrorToString(err));
        }
    }
}

bool WUDP::SendTo(const uint8_t *send_message, uint32_t message_len, const WEndPointInfo &remote) {
    assert(message_len <= MAX_WAN_UDP_PACKAGE_LEN);

    auto [ip, port] = WEndPointInfo::Dump(remote);
    std::cout << "WUDP::SendTo " << ip << " : " << port << " " << this->handler_->socket_ << " "
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
 * WUDPChannel
 ************************************************************/

WUDPChannel::WUDPChannel(const WEndPointInfo &local_ep, const WEndPointInfo &remote_ep, std::weak_ptr<ev_hdle_t> handle) {
    this->local_endpoint_  = local_ep;
    this->remote_endpoint_ = remote_ep;

    auto socket = MakeBindedSocket(local_endpoint_);
    assert(socket != -1);

    std::cout << "WUDPChannel socket " << socket << std::endl;

    bool ok = ConnectToHost(socket, remote_endpoint_);
    // like not be fail
    // if(!ok) {
    //     assert("WUDPChannel ConnectToHost err ");
    //     std::cout << "WUDPChannel ConnectToHost " << socket << std::endl;
    // }

    this->handler_ = ev_hdler_t::CreateHandler(socket, this, handle);
    this->handler_->SetEvents(HandlerEventType::EV_IN);
    this->handler_->Enable();
}

WUDPChannel::~WUDPChannel() {
    if(this->handler_) {
        if(this->handler_->IsEnable())
            this->handler_->DisEnable();
    }
}

void WUDPChannel::ChannelIn() {
    // std::cout << "accpet channel in" << std::endl;

    WEndPointInfo ei;
    uint8_t       buf[MAX_UDP_BUFFER_LEN]{0};

    // auto recv_len = RecvFrom(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, &ei);
    auto recv_len = recv(this->handler_->socket_, buf, MAX_UDP_BUFFER_LEN, 0);

    // if(ei != this->remote_endpoint_) {
    //     std::cout << "not wanted remote " << std::endl;
    //     return;
    // }

    if(recv_len <= 0) { // error
        onErr(GetError());
    } else {
        if(!listener_) {
            return;
        }

        listener_->OnMessage(buf, recv_len);
    }
}

void WUDPChannel::onErr(int err) {
    if(err != 0) {
        if(listener_) {
            listener_->OnError(ErrorToString(err));
        }
    }
}

bool WUDPChannel::Send(const uint8_t *send_message, uint32_t message_len) {
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
 * WChannel
 ************************************************************/

WChannel::WChannel(const WEndPointInfo &local, const WEndPointInfo &remote, std::unique_ptr<ev_hdler_t> h) :
    local_endpoint_(local), remote_endpoint_(remote), event_handler_(std::move(h)) {
    DEBUGADD("WChannel");

    assert(this->event_handler_);

    this->event_handler_->SetEvents(HandlerEventType::EV_IN);
    this->event_handler_->Enable();
}

WChannel::~WChannel() {
    DEBUGRM("WChannel");
    std::cout << "~WChannel" << std::endl;
}

bool WChannel::Init() { return true; }

void WChannel::ShutDown(int how) {
    // std::cout << "WChannel::ShutDown()" << std::endl;

    assert(how >= SHUT_RD);
    assert(how <= SHUT_RDWR);

    ::shutdown(this->event_handler_->socket_, how);
}

void WChannel::CloseChannel() {}

void WChannel::Send(const uint8_t *send_message, uint32_t message_len) {
    assert(message_len <= MAX_CHANNEL_SEND_SIZE);

    // has buf
    if(max_send_buf_size_ != 0 && !send_buf.IsEmpty() > 0) {
        // push data to buf
        auto l = this->send_buf.Write(send_message, message_len);
        if(l != message_len) {
            this->onChannelError(EAGAIN);
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
        this->onChannelError(GetError());
        return;
    }


    if(res < message_len) {
        if(max_send_buf_size_ == 0) {
            this->onChannelError(EAGAIN);
            return;
        }

        auto l = this->send_buf.Write(send_message + res, message_len - (uint32_t)res);
        if(l != (message_len - res)) {
            this->onChannelError(EAGAIN);
            return;
        }

        auto events = this->event_handler_->GetEvents();
        events |= (HandlerEventType::EV_OUT);
        this->event_handler_->SetEvents(events);
    } else {
        // std::cout << "send all " << res << std::endl;
    }
}

void WChannel::SetRecvBufferMaxSize(int page_count, int page_size) {
    this->max_recv_buf_size_ = page_count * page_size;
    assert(this->max_recv_buf_size_ <= MAX_CHANNEL_RECV_BUFFER_SIZE);

    recv_buf.Init(page_count, page_size);
}

void WChannel::SetSendBufferMaxSize(int page_count, int page_size) {
    this->max_send_buf_size_ = page_count * page_size;
    assert(this->max_send_buf_size_ <= MAX_CHANNEL_SEND_BUFFER_SIZE);

    send_buf.Init(page_count, page_size);
}

void WChannel::ChannelIn() {
    assert(this->event_handler_);
    assert(this->max_recv_buf_size_);

    // std::cout << "WChannel ChannelIn this& " << this << std::endl;

    ssize_t recv_len = 0;
    // recv_len         = ::recv(this->client_socket_, this->recv_buf_, this->recv_buf_size_, 0);
    // std::cout << "WChannel::ChannelIn() channel in recv " << recv_len << std::endl;
    recv_len = recv_buf.Write([&](iovec *vec, int len) -> int64_t {
        // std::cout << "WChannel::ChannelIn() channel in iovec len " << len << std::endl;
        auto rcv_len = readv(this->event_handler_->socket_, vec, len);
        // std::cout << "WChannel::ChannelIn() channel in recv " << recv_len << std::endl;

        if(rcv_len == 0) {
            this->onChannelClose();
        } else if(rcv_len < 0) {
            this->onChannelError(GetError());
        }

        return rcv_len;
    });
    if(recv_len <= 0) { // has emit in recv_buf.Write
        std::cout << "WChannel ChannelIn recv_len " << recv_len << std::endl;
        return;
    }


    if(this->listener_ == nullptr) {
        std::cout << "WChannel::ChannelIn() listener is nullptr." << std::endl;
        return;
    }

    recv_buf.Read([&](const uint8_t *msg, uint32_t len) -> int64_t {
        this->listener_->onReceive(msg, len);
        return len;
    });
    assert(this->recv_buf.IsEmpty());

    // std::cout << "WChannel ChannelIn " << std::endl;

    // if(recv_len < 0) {
    //     std::cout << "recv len -1 :" << strerror(errno) << std::endl;
    //     this->onChannelError(errno);
    // } else if(recv_len == 0) {
    //     this->onChannelClose();
    // } else {
    //     this->listener_->onReceive(this->recv_buf_, recv_len);
    //     // std::cout << "WChannel in on read end " << std::endl;
    // }
    // std::cout << "WChannel in end " << std::endl;
}

// can write
void WChannel::ChannelOut() {
    // std::cout << "WChannel::ChannelOut " << std::endl;

    assert(this->event_handler_);

    if(this->max_send_buf_size_ == 0 || this->send_buf.IsEmpty()) {
        auto events = this->event_handler_->GetEvents();
        events &= (!HandlerEventType::EV_OUT);
        this->event_handler_->SetEvents(events);
        return;
    }

    send_buf.Read([&](const iovec *vec, int len) {
        auto send_len = ::writev(this->event_handler_->socket_, vec, len);
        if(send_len < 0) {
            this->listener_->onError(ErrorToString(GetError()));
        }

        return send_len;
    });

    if(this->send_buf.IsEmpty()) {
        auto events = this->event_handler_->GetEvents();
        events      = events & (!HandlerEventType::EV_OUT);
        this->event_handler_->SetEvents(events);
    }

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
    std::cout << "error " << ErrorToString(error_code) << std::endl;
    this->listener_->onError(ErrorToString(error_code));
}


} // namespace wlb::network

