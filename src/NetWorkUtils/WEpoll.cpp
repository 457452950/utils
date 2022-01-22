#include "../../include/WEpoll.h"

#if defined(OS_IS_LINUX)

namespace wlb::NetWork
{

epoll_type CreateNewEpoll()
{
    return epoll_create(1);
}

bool EpollAddSocket(epoll_type epoll, base_socket_type socket, uint32_t events)
{
    struct epoll_event event;
    event.events = events;
    event.data.fd = socket;

    if ( ::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0 ){
        return true;
    }
    return false;
}

bool EpollRemoveSocket(epoll_type epoll, base_socket_type socket)
{
    if ( ::epoll_ctl(epoll, EPOLL_CTL_DEL, socket, nullptr) == 0 )
    {
        return true;
    }
    return false;
}

int32_t EpollGetEvents(epoll_type epoll, struct epoll_event * events, int32_t events_size)
{
    if (events_size <= 0)
    {
        return 0;
    }
    
    return ::epoll_wait(epoll, events, events_size, 0);
}

void CloseEpoll(epoll_type epoll)
{
    ::close(epoll);
}


WEpoll::~WEpoll()
{
    this->Close();
}

bool WEpoll::Init()
{
    this->_epoll = CreateNewEpoll();
    if (this->_epoll == -1)
    {
        return false;
    }
    return true;
}

void WEpoll::Close()
{
    if (this->_epoll != -1)
    {
        CloseEpoll(this->_epoll);
        this->_epoll = -1;
    }
}

bool WEpoll::AddSocket(base_socket_type socket, uint32_t events)
{
    return EpollAddSocket(this->_epoll, socket, events);
}

void WEpoll::RemoveSocket(base_socket_type socket)
{
    EpollRemoveSocket(this->_epoll, socket);
}

int32_t WEpoll::GetEvents(epoll_event * events, int32_t events_size)
{
    return EpollGetEvents(this->_epoll, events, events_size);
}


} // namespace wlb::NetWork

#endif // OS_IS_LINUX
