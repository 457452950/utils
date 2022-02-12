#pragma once

#include "./WNetWorkUtils.h"

#if defined(OS_IS_LINUX)

#include <map>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include "./WNetWorkHandler.hpp"
#include "./WTimerHandler.hpp"

namespace wlb
{
namespace NetWork
{

using epoll_type = int32_t;
using epoll_ptr = epoll_type*;



epoll_type CreateNewEpoll();

// events

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
bool EpollAddSocket(epoll_type epoll, base_socket_type socket, uint32_t events);
bool EpollModifySocket(epoll_type epoll, base_socket_type socket, uint32_t events);
bool EpollRemoveSocket(epoll_type epoll, base_socket_type socket);

// 
// struct epoll_event
// {
//   uint32_t events;	/* Epoll events */
//   epoll_data_t data;	/* User data variable */
// } __EPOLL_PACKED;

// return 0 No Events
// return -1 errno
int32_t EpollGetEvents(epoll_type epoll, struct epoll_event * events, int32_t events_size);

void CloseEpoll(epoll_type epoll);

class WBaseEpoll
{
public:
    explicit WBaseEpoll(/* args */) = default;
    WBaseEpoll(const WBaseEpoll& other) = delete;
    WBaseEpoll& operator=(const WBaseEpoll& other) = delete;
    virtual ~WBaseEpoll();

    virtual bool Init();
    virtual void Close();

    virtual bool AddSocket(base_socket_type socket, uint32_t events);
    virtual bool ModifySocket(base_socket_type socket, uint32_t events);
    virtual void RemoveSocket(base_socket_type socket);

    virtual int32_t GetEvents(epoll_event* events, int32_t events_size);

protected:
    epoll_type _epoll{-1};

};



class WEpoll : public WBaseEpoll, public WNetWorkHandler
{
public: 
    explicit WEpoll() {};
    virtual ~WEpoll() = default;

    bool Init(uint32_t events_size);
    void Close();
    void GetAndEmitEvents();

    bool AddSocket(WNetWorkHandler::Listener* listener, base_socket_type socket, uint32_t op) override;
    bool ModifySocket(base_socket_type socket, uint32_t op) override;
    void RemoveSocket(base_socket_type socket) override;

private:
    uint32_t GetEpollEventsFromOP(uint32_t op);

protected:
    std::map<base_socket_type, WNetWorkHandler::Listener*> _listeners;

    epoll_event * _events{nullptr};
    int32_t _events_size{0};

    const int32_t default_events_size{128};

private:
    int32_t GetEvents(epoll_event* events, int32_t events_size) { return 0; };

};



////////////////////////////////
// timer
////////////////////////////////

// timerfd = socket fd

class WTimerEpoll : public WBaseEpoll, public WTimerHandler
{
public:
    WTimerEpoll() = default;
    virtual ~WTimerEpoll() {};
    
    // override WTimerHandler
    bool Init() override;
    void Close() override;

    void GetAndEmitTimer() override;
    void AddTimer(Listener* listener, timerfd timer) override;
    void RemoveTimer(timerfd timer) override;

private:
    std::map<timerfd, WTimerHandler::Listener*> _listeners;

    epoll_event * _events{nullptr};
    int32_t _events_size{30};
    const int32_t default_events_size{30};
};




}   // namespace NetWork

} // namespace wlb


#endif
