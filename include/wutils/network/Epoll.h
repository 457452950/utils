#pragma once
#ifndef UTILS_EPOLL_H
#define UTILS_EPOLL_H

#include <atomic>
#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>

#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "Event.h"
#include "NetWorkUtils.h"


namespace wutils::network {

using epoll_type = int32_t;
using epoll_ptr  = epoll_type *;

epoll_type CreateNewEpollFd();

//    epoll events

//    EPOLLIN
//           The associated file is available for read(2) operations.

//    EPOLLOUT
//           The associated file is available for write(2) operations.

//    EPOLLRDHUP (since Linux 2.6.17)
//           Stream socket peer closed connection, or shut down writing  half
//           of connection.  (This flag is especially useful for writing sim‐
//           ple code to detect peer shutdown when using Edge Triggered moni‐
//           toring.)

//    EPOLLPRI
//           There  is  an exceptional condition on the file descriptor.  See
//           the discussion of POLLPRI in poll(2).

//    EPOLLERR
//           Error condition happened  on  the  associated  file  descriptor.
//           This event is also reported for the write end of a pipe when the
//           read end has been closed.  epoll_wait(2) will always report  for
//           this event; it is not necessary to set it in events.

//    EPOLLHUP
//               Hang   up   happened   on   the   associated   file  descriptor.
//               epoll_wait(2) will always wait for this event; it is not  neces‐
//               sary to set it in events.

//               Note that when reading from a channel such as a pipe or a stream
//               socket, this event merely indicates that the peer closed its end
//               of the channel.  Subsequent reads from the channel will return 0
//               (end of file) only after all outstanding data in the channel has
//               been consumed.

//    EPOLLET
//               Sets  the  Edge  Triggered  behavior for the associated file de‐
//               scriptor.  The default behavior for epoll  is  Level  Triggered.
//               See  epoll(7) for more detailed information about Edge and Level
//               Triggered event distribution architectures.

//    EPOLLONESHOT (since Linux 2.6.2)
//               Sets the one-shot behavior for the associated  file  descriptor.
//               This  means that after an event is pulled out with epoll_wait(2)
//               the associated file descriptor is  internally  disabled  and  no
//               other  events will be reported by the epoll interface.  The user
//               must call epoll_ctl() with EPOLL_CTL_MOD to rearm the  file  de‐
//               scriptor with a new event mask.

//    EPOLLWAKEUP (since Linux 3.5)
//               If  EPOLLONESHOT  and  EPOLLET are clear and the process has the
//               CAP_BLOCK_SUSPEND capability, ensure that the  system  does  not
//               enter  "suspend"  or  "hibernate" while this event is pending or
//               being processed.  The event is considered as  being  "processed"
//               from the time when it is returned by a call to epoll_wait(2) un‐
//               til the next call to epoll_wait(2) on the same epoll(7) file de‐
//               scriptor,  the  closure  of that file descriptor, the removal of
//               the event file descriptor with EPOLL_CTL_DEL, or the clearing of
//               EPOLLWAKEUP  for  the  event file descriptor with EPOLL_CTL_MOD.
//               See also BUGS.

//    EPOLLEXCLUSIVE (since Linux 4.5)
//               Sets an exclusive wakeup mode for the epoll file descriptor that
//               is  being  attached  to  the target file descriptor, fd.  When a
//               wakeup event occurs and multiple epoll file descriptors are  at‐
//               tached to the same target file using EPOLLEXCLUSIVE, one or more
//               of the  epoll  file  descriptors  will  receive  an  event  with
//               epoll_wait(2).   The  default in this scenario (when EPOLLEXCLU‐
//               SIVE is not set) is for all epoll file descriptors to receive an
//               event.   EPOLLEXCLUSIVE  is  thus useful for avoiding thundering
//               herd problems in certain scenarios.

//               If the same file descriptor is in multiple epoll instances, some
//               with  the  EPOLLEXCLUSIVE  flag, and others without, then events
//               will be provided to all epoll instances  that  did  not  specify
//               EPOLLEXCLUSIVE, and at least one of the epoll instances that did
//               specify EPOLLEXCLUSIVE.

//               The following  values  may  be  specified  in  conjunction  with
//               EPOLLEXCLUSIVE:  EPOLLIN,  EPOLLOUT,  EPOLLWAKEUP,  and EPOLLET.
//               EPOLLHUP and EPOLLERR can also be specified, but this is not re‐
//               quired:  as  usual, these events are always reported if they oc‐
//               cur, regardless of whether they are specified  in  events.   At‐
//               tempts to specify other values in events yield the error EINVAL.

//               EPOLLEXCLUSIVE  may  be used only in an EPOLL_CTL_ADD operation;
//               attempts to employ it with EPOLL_CTL_MOD  yield  an  error.   If
//               EPOLLEXCLUSIVE has been set using epoll_ctl(), then a subsequent
//               EPOLL_CTL_MOD on the same epfd, fd pair yields an error.  A call
//               to epoll_ctl() that specifies EPOLLEXCLUSIVE in events and spec‐
//               ifies the target file descriptor fd as an  epoll  instance  will
//               likewise fail.  The error in all of these cases is EINVAL.
////////////////////////////////////////////////////////////////////////////////////


bool EpollAddSocket(epoll_type epoll, socket_t socket, uint32_t events);
bool EpollModifySocket(epoll_type epoll, socket_t socket, uint32_t events);
bool EpollRemoveSocket(epoll_type epoll, socket_t socket);

bool EpollAddSocket(epoll_type epoll, socket_t socket, uint32_t events, epoll_data_t data);
bool EpollModifySocket(epoll_type epoll, socket_t socket, uint32_t events, epoll_data_t data);

// struct epoll_event
// {
//   uint32_t events;	/* Epoll events */
//   epoll_data_t data;	/* User data variable */
// } __EPOLL_PACKED;

// return 0 No Events
// return -1 errno
int32_t EpollGetEvents(epoll_type epoll, struct epoll_event *events, int32_t events_size, int32_t timeout = 0);
void    CloseEpoll(epoll_type epoll);

// not thread safe
class BaseEpoll final {
public:
    BaseEpoll();
    ~BaseEpoll();
    // no copyable
    BaseEpoll(const BaseEpoll &other)            = delete;
    BaseEpoll &operator=(const BaseEpoll &other) = delete;

    bool Init();
    void Close();

    bool AddSocket(socket_t socket, uint32_t events);
    bool AddSocket(socket_t socket, uint32_t events, epoll_data_t data);
    bool ModifySocket(socket_t socket, uint32_t events);
    bool ModifySocket(socket_t socket, uint32_t events, epoll_data_t data);
    bool RemoveSocket(socket_t socket);


    int32_t GetEvents(epoll_event *events, int32_t events_size, int32_t timeout = 0);

protected:
    epoll_type epoll_fd_{-1};
};


template <typename UserData>
class Epoll final : public EventHandle<UserData> {
public:
    Epoll();
    ~Epoll();

    using EventHandler = typename EventHandle<UserData>::EventHandler;


    // control
    bool AddSocket(EventHandler *handler) override;
    bool ModifySocket(EventHandler *handler) override;
    void DelSocket(EventHandler *handler) override;

    bool Init() override;
    void Loop() override;
    void Stop() override;
    void Wake();

private:
    static uint32_t ParseToEpollEvent(uint8_t events);
    void            EventLoop();

private:
    BaseEpoll ep;
    uint32_t  fd_count_{0};
    int       wakeup_fd_{-1}; // event_fd
    bool      active_{false};
};


template <typename UserData>
Epoll<UserData>::Epoll() = default;

template <typename UserData>
Epoll<UserData>::~Epoll() {
    this->Stop();

    ::close(wakeup_fd_);

    ep.Close();
}


template <typename UserData>
bool Epoll<UserData>::Init() {
    // DONE: 独立出Init函数
    if(!ep.Init()) {
        // std::cerr << "epoll init failed!" << std::endl;
        return false;
    }

    wakeup_fd_ = eventfd(0, 0);
    if(wakeup_fd_ == -1) {
        return false;
    }

    if(this->ep.AddSocket(wakeup_fd_, EPOLLIN) == false) {
        return false;
    }

    return true;
}


template <typename UserData>
bool Epoll<UserData>::AddSocket(EventHandler *handler) {
    epoll_data_t ep_data;

    ep_data.ptr = handler;
    auto ev     = this->ParseToEpollEvent(handler->GetEvents());

    if(!ep.AddSocket(handler->socket_, ev, ep_data)) {
        return false;
    }

    ++this->fd_count_;

    return true;
}

template <typename UserData>
bool Epoll<UserData>::ModifySocket(EventHandler *handler) {
    epoll_data_t ep_data;

    ep_data.ptr = handler->user_data_;
    auto ev     = this->ParseToEpollEvent(handler->GetEvents());

    return ep.ModifySocket(handler->socket_, ev, ep_data);
}

template <typename UserData>
void Epoll<UserData>::DelSocket(EventHandler *handler) {

    if(!this->ep.RemoveSocket(handler->socket_)) {
        return;
    }

    --this->fd_count_;
}

template <typename UserData>
void Epoll<UserData>::Loop() {
    this->active_ = true;
    this->EventLoop();
}

template <typename UserData>
inline void Epoll<UserData>::Stop() {
    this->active_ = false;
    this->Wake();
}

template <typename UserData>
inline void Epoll<UserData>::Wake() {
    eventfd_write(this->wakeup_fd_, 1);
}

template <typename UserData>
uint32_t Epoll<UserData>::ParseToEpollEvent(uint8_t events) {
    uint32_t ev = 0;
    if(events & HandlerEventType::EV_IN) {
        ev |= EPOLLIN;
    }
    if(events & HandlerEventType::EV_OUT) {
        ev |= EPOLLOUT;
    }
    return ev;
}


template <typename UserData>
void Epoll<UserData>::EventLoop() {

    std::unique_ptr<epoll_event[]> events;

    while(this->active_) {
        int events_size = fd_count_;
        events.reset(new epoll_event[events_size]);

        events_size = ep.GetEvents(events.get(), events_size, -1);

        if(!this->active_) {
            std::cout << "close !!!" << std::endl;
            uint64_t cnt;
            eventfd_read(this->wakeup_fd_, &cnt);
            assert(cnt == 1);
            break;
        }

        if(events_size == -1) {
            break;
        } else if(events_size == 0) {
            // TODO:add timeout event
            continue;
        }

        for(int i = 0; i < events_size; ++i) {

            if(!this->active_) {
                break;
            }

            uint32_t ev   = events[i].events;
            auto    *data = (typename EventHandle<UserData>::EventHandler *)events[i].data.ptr;

            assert(data);
            socket_t sock = data->socket_;
            uint8_t  eev  = data->GetEvents();

            assert(sock > 0);

            // FIXME: write_ 中可能释放 data
            if(ev & EPOLLOUT && eev & HandlerEventType::EV_OUT) {
                if(this->write_) {
                    this->write_(sock, data->user_data_);
                }
            }
            if(ev & EPOLLIN && eev & HandlerEventType::EV_IN) {
                if(this->read_) {
                    this->read_(sock, data->user_data_);
                }
            }
        }
    }
}


} // namespace wutils::network

#endif // UTILS_EPOLL_H
