#include "WNetWork/WEpoll.hpp"
#include <iostream>

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
    event.data.fd = socket;
    event.events = events;

    if ( ::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0 ){
        
        return true;
    }
    
    return false;
}

bool EpollModifySocket(epoll_type epoll, base_socket_type socket, uint32_t events)
{
    struct epoll_event event;
    event.data.fd = socket;
    event.events = events;

    if ( ::epoll_ctl(epoll, EPOLL_CTL_MOD, socket, &event) == 0 ){
        
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
    
    return ::epoll_wait(epoll, events, events_size, 1);
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
    if (this->_epoll == -1)
    {
        
        return false;
    }
    
    if (socket == -1)
    {
        /* code */
    }
    
    
    return EpollAddSocket(this->_epoll, socket, events);
}

bool WBaseEpoll::ModifySocket(base_socket_type socket, uint32_t events)
{
    return EpollModifySocket(this->_epoll, socket, events);
}

void WBaseEpoll::RemoveSocket(base_socket_type socket)
{
    EpollRemoveSocket(this->_epoll, socket);
}

int32_t WBaseEpoll::GetEvents(epoll_event * events, int32_t events_size)
{
    return EpollGetEvents(this->_epoll, events, events_size);
}



////////////////////////////////////////////////////////////////////////
// WEpoll

bool WEpoll::Init(uint32_t events_size)
{
    this->_listeners.clear();

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
        
        return;
    }
    
    int32_t curr_events_size = WBaseEpoll::GetEvents(_events, this->_events_size);

    if (curr_events_size == -1)
    {
        // error
    }
    else
    {
        Listener* listener;
        for (int32_t index = 0; index < curr_events_size; ++index)
        {
            base_socket_type socket = _events[index].data.fd;
            auto it = this->_listeners.find(socket);
            if (it == this->_listeners.end())
            {
                // cant find listener
                return;
            }
            listener = it->second;

            
            if (_events[index].events & EPOLLHUP)  // 对端已经关闭 受到最后一次挥手
            {
                
            }
            if (_events[index].events & EPOLLERR)
            {
                
            }
            if (_events[index].events & EPOLLRDHUP)    // 对端关闭写，
            {
                
            }
            if (_events[index].events & EPOLLIN)
            {
                
            }
            if (_events[index].events & EPOLLOUT)
            {
                
            }

            if (_events[index].events & EPOLLHUP)  // 对端已经关闭 受到最后一次挥手
            {
                listener->OnClosed();
                RemoveSocket(socket);
            }
            else if (_events[index].events & EPOLLERR)
            {
                listener->OnError(errno);
            }
            else if (_events[index].events & EPOLLRDHUP)    // 对端关闭写，
            {
                // peer shutdown write
                listener->OnShutdown();
            }
            else if (_events[index].events & EPOLLIN)
            {
                listener->OnRead();
            }
            else if (_events[index].events & EPOLLOUT)
            {
                listener->OnWrite();
            }
            
        }

        this->_events_size > curr_events_size ?
                this->_events_size = 10 + (curr_events_size / 10 + this->_events_size * 9 / 10):
                this->_events_size = curr_events_size * 1.5;
        
        // 
    }
    
    delete[] this->_events;
}

void WEpoll::Close()
{
    if (this->_events != nullptr)
    {
        delete [] this->_events;
        this->_events = nullptr;
    }
    this->_listeners.clear();
    WBaseEpoll::Close();
}

bool WEpoll::AddSocket(WNetWorkHandler::Listener* listener, 
                        base_socket_type socket, 
                        uint32_t op)
{
    this->_listeners.insert(std::make_pair(socket, listener));

    uint32_t _event = 0;
    _event = GetEpollEventsFromOP(op);
    return WBaseEpoll::AddSocket(socket, _event);
}

bool WEpoll::ModifySocket(base_socket_type socket, uint32_t op)
{
    uint32_t _event = 0;
    _event = GetEpollEventsFromOP(op);
    return WBaseEpoll::ModifySocket(socket, _event);
}

void WEpoll::RemoveSocket(base_socket_type socket)
{
    WBaseEpoll::RemoveSocket(socket);
    this->_listeners.erase(socket);
}

uint32_t WEpoll::GetEpollEventsFromOP(uint32_t op)
{
    uint32_t _events = 0;
    if (op & OP_IN)
    {
        _events |= EPOLLIN;
    }
    if (op & OP_OUT)
    {
        _events |= EPOLLOUT;
    }
    if (op & OP_ERR)
    {
        _events |= EPOLLERR;
    }
    if (op & OP_SHUT)
    {
        _events |= EPOLLRDHUP;
    }
    if (op & OP_CLOS)
    {
        _events |= EPOLLHUP;
    }
    
    return _events;
}


/////////////////////////////////
// WTimerEpoll

bool WTimerEpoll::Init()
{
    if ( !WBaseEpoll::Init())
    {
        return false;
    }
    
    return true;
}

void WTimerEpoll::Close()
{
    WBaseEpoll::Close();
    this->_listeners.clear();
}

void WTimerEpoll::Destroy()
{
    if (this->_events != nullptr)
    {
        delete [] this->_events;
        this->_events = nullptr;
    }
    
}

void WTimerEpoll::GetAndEmitTimer()
{
    this->_events = new(std::nothrow) epoll_event[this->_events_size];
    if (_events == nullptr)
    {
        return;
    }
    
    int32_t curr_events_size = WBaseEpoll::GetEvents(_events, this->_events_size);

    if (curr_events_size == -1)
    {
        // error
        std::cout << "error no:" << errno << " " << strerror(errno) << std::endl;
    }
    else
    {
        Listener* listener;
        for (int32_t index = 0; index < curr_events_size; ++index)
        {
            timerfd timer = _events[index].data.fd;
            auto it = this->_listeners.find(timer);
            if (it == this->_listeners.end())
            {
                // cant find listener
                return;
            }
            listener = it->second;

            

            if (_events[index].events & EPOLLIN)
            {
                listener->OnTime(timer);
                uint64_t exp = 0;
                read(timer, &exp, sizeof(uint64_t));
            }
        }

        this->_events_size > curr_events_size ?
                this->_events_size = 10 + (curr_events_size / 10 + this->_events_size * 9 / 10):
                this->_events_size = curr_events_size * 1.5;
        
        // 
    }
    
    delete[] this->_events;
}

void WTimerEpoll::AddTimer(Listener* listener, timerfd timer)
{
    this->_listeners.insert(std::make_pair(timer, listener));
    uint32_t event = 0;
    event |= EPOLLET;
    event |= EPOLLIN;
    WBaseEpoll::AddSocket(timer, event);
}

void WTimerEpoll::RemoveTimer(timerfd timer)
{
    WBaseEpoll::RemoveSocket(timer);
    this->_listeners.erase(timer);
}


} // namespace wlb::NetWork

#endif // OS_IS_LINUX
