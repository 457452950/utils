#pragma once
#ifndef UTILS_EPOLL_H
#define UTILS_EPOLL_H

#include <atomic>
#include <cassert>
#include <functional>
#include <iostream>
#include <list>

#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "Tools.h"
#include "wutils/network/io_event/IOContext.h"


namespace wutils::network {

using epoll_type = int32_t;
using epoll_ptr  = epoll_type *;

HEAD_ONLY epoll_type CreateNewEpollFd() { return epoll_create(1); }

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


HEAD_ONLY bool EpollAddSocket(epoll_type epoll, socket_t socket, uint32_t events) {
    struct epoll_event event {};
    event.data.fd = socket;
    event.events  = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0) {
        return true;
    }

    return false;
}

HEAD_ONLY bool EpollModifySocket(epoll_type epoll, socket_t socket, uint32_t events) {
    struct epoll_event event {};
    event.data.fd = socket;
    event.events  = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_MOD, socket, &event) == 0) {
        return true;
    }

    return false;
}

HEAD_ONLY bool EpollRemoveSocket(epoll_type epoll, socket_t socket) {
    if(::epoll_ctl(epoll, EPOLL_CTL_DEL, socket, nullptr) == 0) {
        return true;
    }
    return false;
}

HEAD_ONLY bool EpollAddSocket(epoll_type epoll, socket_t socket, uint32_t events, epoll_data_t data) {
    struct epoll_event event {};
    event.data   = data;
    event.events = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0) {
        return true;
    }

    return false;
}

HEAD_ONLY bool EpollModifySocket(epoll_type epoll, socket_t socket, uint32_t events, epoll_data_t data) {
    struct epoll_event event {};
    event.data   = data;
    event.events = events;

    if(::epoll_ctl(epoll, EPOLL_CTL_MOD, socket, &event) == 0) {
        return true;
    }

    return false;
}

// struct epoll_event
// {
//   uint32_t events;	/* EpollContext events */
//   epoll_data_t data;	/* User data variable */
// } __EPOLL_PACKED;

/**
 *
 * @param epoll
 * @param events
 * @param events_size
 * @param timeout
 * @return 0 for time out . -1 for error , check errno. number for event count.
 */
HEAD_ONLY int32_t EpollGetEvents(epoll_type epoll, struct epoll_event *events, int32_t events_size, int32_t timeout) {
    assert(events_size > 0);

    return ::epoll_wait(epoll, events, events_size, timeout);
}

HEAD_ONLY void CloseEpoll(epoll_type epoll) { ::close(epoll); }

class Epoll final {
public:
    Epoll() = default;
    ~Epoll() { this->Close(); };
    // no copyable
    Epoll(const Epoll &other)            = delete;
    Epoll &operator=(const Epoll &other) = delete;

    bool Init() {
        this->epoll_fd_ = CreateNewEpollFd();
        if(this->epoll_fd_ == INVALID_SOCKET) {
            return false;
        }
        return true;
    }
    void Close() {
        if(this->epoll_fd_ != INVALID_SOCKET) {
            CloseEpoll(this->epoll_fd_);
            this->epoll_fd_ = INVALID_SOCKET;
        }
    }

    bool AddSocket(socket_t socket, uint32_t events) {
        assert(socket != INVALID_SOCKET);
        assert(events > 0);

        if(!EpollAddSocket(this->epoll_fd_, socket, events)) {
            return false;
        }
        return true;
    }
    bool AddSocket(socket_t socket, uint32_t events, epoll_data_t data) {
        assert(socket != -1);
        assert(events > 0);

        if(!EpollAddSocket(this->epoll_fd_, socket, events, data)) {
            return false;
        }
        return true;
    }

    bool ModifySocket(socket_t socket, uint32_t events) {
        assert(socket != -1);

        if(!EpollModifySocket(this->epoll_fd_, socket, events)) {
            return false;
        }
        return true;
    }
    bool ModifySocket(socket_t socket, uint32_t events, epoll_data_t data) {
        assert(socket != -1);
        assert(events > 0);

        if(!EpollModifySocket(this->epoll_fd_, socket, events, data)) {
            return false;
        }
        return true;
    }

    bool RemoveSocket(socket_t socket) {
        assert(socket != -1);
        return EpollRemoveSocket(this->epoll_fd_, socket);
    }

    int32_t GetEvents(epoll_event *events, int32_t events_size, int32_t timeout = 0) {
        auto res = EpollGetEvents(this->epoll_fd_, events, events_size, timeout);
        return res;
    }

protected:
    epoll_type epoll_fd_{INVALID_SOCKET};
};


} // namespace wutils::network

#endif // UTILS_EPOLL_H
