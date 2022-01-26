#include "../../include/WEpoll.hpp"

#if defined(OS_IS_LINUX)

namespace wlb::NetWork
{

epoll_type CreateNeWBaseEpoll()
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


WBaseEpoll::~WBaseEpoll()
{
    this->Close();
}

bool WBaseEpoll::Init()
{
    this->_epoll = CreateNeWBaseEpoll();
    if (this->_epoll == -1)
    {
        return false;
    }
    return true;
}

void WBaseEpoll::Close()
{
    if (this->_epoll != -1)
    {
        CloseEpoll(this->_epoll);
        this->_epoll = -1;
    }
}

bool WBaseEpoll::AddSocket(base_socket_type socket, uint32_t events)
{
    return EpollAddSocket(this->_epoll, socket, events);
}

void WBaseEpoll::RemoveSocket(base_socket_type socket)
{
    EpollRemoveSocket(this->_epoll, socket);
}

int32_t WBaseEpoll::GetEvents(epoll_event * events, int32_t events_size)
{
    return EpollGetEvents(this->_epoll, events, events_size);
}


bool WEpoll::Init(uint32_t events_size)
{
    if ( ! WBaseEpoll::Init() )
    {
        return false;
    }

    events_size < default_events_size ? 
            this->_events_size = default_events_size : 
            this->_events_size = events_size;

    return true;
}

void WEpoll::GetAndEmitEvents()
{
    this->_events = new(std::nothrow) epoll_event[this->_events_size];
    if (_events == nullptr)
    {
        // error
        return;
    }
    

    int32_t curr_events_size = WBaseEpoll::GetEvents(_events, this->_events_size);

    if (curr_events_size == -1)
    {
        // error
    }
    else
    {
        for (int32_t index = 0; index < curr_events_size; ++index)
        {
            if (_events[index].events & EPOLLIN)
            {
                _listener->OnRead(_events[index].data.fd);
            }
            else if (_events[index].events & EPOLLOUT)
            {
                _listener->OnWrite(_events[index].data.fd);
            }
            else if (_events[index].events & EPOLLERR)
            {
                _listener->OnError(_events[index].data.fd, std::string(strerror(errno)));
            }
            else if (_events[index].events & EPOLLHUP)  // 对端已经关闭 受到最后一次挥手
            {
                _listener->OnClosed(_events[index].data.fd);
            }
            else if (_events[index].events & EPOLLRDHUP)    // 对端关闭写，
            {
                // peer shutdown write
            }
        }

        this->_events_size > curr_events_size ?
                this->_events_size = curr_events_size / 10 + this->_events_size * 9 / 10 :
                this->_events_size = curr_events_size * 1.5;
    }
    
    delete[] this->_events;
}

void WEpoll::Close()
{
    if (this->_events != nullptr)
    {
        delete [] this->_events;
    }
    WBaseEpoll::Close();
}

bool WEpoll::AddSocket(base_socket_type socket, uint32_t op)
{
    uint32_t _event = 0;
    _event = GetEpollEventsFromOP(op);
    return AddSocket(socket, _event);
}

bool WEpoll::ModifySocket(base_socket_type socket, uint32_t op)
{
    uint32_t _event = 0;
    _event = GetEpollEventsFromOP(op);
    return ModifySocket(socket, _event);
}

void WEpoll::RemoveSocket(base_socket_type socket)
{
    WBaseEpoll::RemoveSocket(socket);
}

uint32_t WEpoll::GetEpollEventsFromOP(uint32_t op)
{
    uint32_t _events = 0;
    if (op & OP_IN)
    {
        _events | EPOLLIN;
    }
    if (op & OP_OUT)
    {
        _events | EPOLLOUT;
    }
    if (op & OP_ERR)
    {
        _events | EPOLLERR;
    }
    if (op & OP_SHUT)
    {
        _events | EPOLLRDHUP;
    }
    if (op & OP_CLOS)
    {
        _events | EPOLLHUP;
    }
    
    return _events;
}


} // namespace wlb::NetWork

#endif // OS_IS_LINUX
