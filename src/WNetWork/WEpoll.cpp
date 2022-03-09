#include "WNetWork/WEpoll.hpp"
#include <iostream>
#include "WDebugger.hpp"

#if defined(OS_IS_LINUX)

namespace wlb::NetWork
{

using namespace wlb::debug;

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

bool EpollAddSocket(epoll_type epoll, base_socket_type socket, uint32_t events, epoll_data_t data)
{
    struct epoll_event event;
    event.data = data;
    event.events = events;

    if ( ::epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event) == 0 ){
        return true;
    }
    
    return false;
}
bool EpollModifySocket(epoll_type epoll, base_socket_type socket, uint32_t events, epoll_data_t data)
{
    struct epoll_event event;
    event.data = data;
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

int32_t EpollGetEvents(epoll_type epoll, struct epoll_event * events, int32_t events_size, int32_t timeout)
{
    if (events_size <= 0)
    {
        return 0;
    }
    
    return ::epoll_wait(epoll, events, events_size, timeout);
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
        this->_errorMessage = strerror(errno);
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
        this->_errorMessage = "epollfd cant be -1";
        return false;
    }
    
    if (socket == -1)
    {
        this->_errorMessage = "socket cant be -1";
        return false;
    }
    
    if (!EpollAddSocket(this->_epoll, socket, events))
    {
        this->_errorMessage = strerror(errno);
        return false;
    }
    return true;
}

bool WBaseEpoll::ModifySocket(base_socket_type socket, uint32_t events)
{
    if (this->_epoll == -1)
    {
        this->_errorMessage = "epollfd cant be -1";
        return false;
    }
    
    if (socket == -1)
    {
        this->_errorMessage = "socket cant be -1";
        return false;
    }
    
    if (!EpollModifySocket(this->_epoll, socket, events))
    {
        this->_errorMessage = strerror(errno);
        return false;
    }
    return true;
}
bool WBaseEpoll::AddSocket(base_socket_type socket, uint32_t events, epoll_data_t data)
{
    if (this->_epoll == -1)
    {
        this->_errorMessage = "epollfd cant be -1";
        return false;
    }
    
    if (socket == -1)
    {
        this->_errorMessage = "socket cant be -1";
        return false;
    }
    
    if (!EpollAddSocket(this->_epoll, socket, events, data))
    {
        this->_errorMessage = strerror(errno);
        return false;
    }
    return true;
}

bool WBaseEpoll::ModifySocket(base_socket_type socket, uint32_t events, epoll_data_t data)
{
    if (this->_epoll == -1)
    {
        this->_errorMessage = "epollfd cant be -1";
        return false;
    }
    
    if (socket == -1)
    {
        this->_errorMessage = "socket cant be -1";
        return false;
    }
    
    if (!EpollModifySocket(this->_epoll, socket, events, data))
    {
        this->_errorMessage = strerror(errno);
        return false;
    }
    return true;
}

void WBaseEpoll::RemoveSocket(base_socket_type socket)
{
    if (this->_epoll == -1)
    {
        this->_errorMessage = "epollfd cant be -1";
        return;
    }
    
    if (socket == -1)
    {
        this->_errorMessage = "socket cant be -1";
        return;
    }
    
    if (!EpollRemoveSocket(this->_epoll, socket))
    {
        this->_errorMessage = strerror(errno);
    }
    return;
}

int32_t WBaseEpoll::GetEvents(epoll_event * events, int32_t events_size, int32_t timeout)
{
    auto res = EpollGetEvents(this->_epoll, events, events_size, timeout);
    if ( res == -1)
    {
        this->_errorMessage = strerror(errno);
        return -1;
    }
    return res;
}



////////////////////////////////////////////////////////////////////////
// WEpoll

bool WEpoll::Init(uint32_t events_size)
{
    if ( !WBaseEpoll::Init() )
    {
        return false;
    }

    events_size < default_events_size ? 
            this->_events_size = default_events_size : 
            this->_events_size = events_size;
    
    return true;
}

void WEpoll::GetAndEmitEvents(int32_t timeout)
{
    this->_events = new(std::nothrow) epoll_event[this->_events_size];
    NEWADD;
    if (_events == nullptr)
    {
        this->_errorMessage = "new epoll_event faile ";
        return;
    }
    
    int32_t curr_events_size = WBaseEpoll::GetEvents(_events, this->_events_size, timeout);

    if (curr_events_size == -1)
    {
        this->_errorMessage = strerror(errno);
        return;
    }
    else
    {
        for (int32_t index = 0; index < curr_events_size; ++index)
        {
            WHandlerData* data = (WHandlerData*)_events[index].data.ptr;
            WNetWorkHandler::Listener* listener = data->listener;
            base_socket_type sock = data->socket;
            
            // if (_events[index].events & EPOLLHUP)  // 对端已经关闭 受到最后一次挥手
            // {
                
            // }
            // if (_events[index].events & EPOLLERR)
            // {
                
            // }
            // if (_events[index].events & EPOLLRDHUP)    // 对端关闭写，
            // {
                
            // }
            // if (_events[index].events & EPOLLIN)
            // {
                
            // }
            // if (_events[index].events & EPOLLOUT)
            // {
                
            // }

            if (_events[index].events & EPOLLHUP)  // 对端已经关闭 受到最后一次挥手
            {
                listener->OnClosed();
                RemoveSocket(sock);
            }
            else if (_events[index].events & EPOLLERR)
            {
                listener->OnError(errno);
                RemoveSocket(sock);
            }
            else if (_events[index].events & EPOLLRDHUP)    // 对端关闭写
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
    DELADD;
}

void WEpoll::Close()
{
    if (this->_events != nullptr)
    {
        delete [] this->_events;
        DELADD;
        this->_events = nullptr;
    }
    WBaseEpoll::Close();
}

bool WEpoll::AddSocket(WHandlerData* data, uint32_t op)
{
    epoll_data_t _data;
    _data.ptr = data;

    uint32_t _event = 0;
    _event = GetEpollEventsFromOP(op);
    return WBaseEpoll::AddSocket(data->socket, _event, _data);
}

bool WEpoll::ModifySocket(WHandlerData* data, uint32_t op)
{
    epoll_data_t _data;
    _data.ptr = data;

    uint32_t _event = 0;
    _event = GetEpollEventsFromOP(op);
    return WBaseEpoll::ModifySocket(data->socket, _event, _data);
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
}

void WTimerEpoll::Destroy()
{
    if (this->_events != nullptr)
    {
        delete [] this->_events;
        this->_events = nullptr;
    }
    
}

void WTimerEpoll::GetAndEmitTimer(int32_t timeout)
{
    this->_events = new(std::nothrow) epoll_event[this->_events_size];
    NEWADD;
    if (_events == nullptr)
    {
        return;
    }
    
    int32_t curr_events_size = WBaseEpoll::GetEvents(_events, this->_events_size, timeout);

    if (curr_events_size == -1)
    {
        // error
        std::cout << "error no:" << errno << " " << strerror(errno) << std::endl;
        this->_errorMessage;
    }
    else
    {
        Listener* listener;
        WTimer* timer;
        timerfd fd;
        for (int32_t index = 0; index < curr_events_size; ++index)
        {
            WTimerHandlerData* data = (WTimerHandlerData*)_events[index].data.ptr;
            listener = data->listener;
            timer = data->_timer;
            fd = data->_timerfd;
            
            if (_events[index].events & EPOLLIN)
            {
                listener->OnTime(timer);
                uint64_t exp = 0;
                read(fd, &exp, sizeof(uint64_t));
            }
        }

        this->_events_size > curr_events_size ?
                this->_events_size = 10 + (curr_events_size / 10 + this->_events_size * 9 / 10):
                this->_events_size = curr_events_size * 1.5;
        
        // 
    }
    
    delete[] this->_events;
    DELADD;
}

void WTimerEpoll::AddTimer(WTimerHandlerData* data)
{
    epoll_data_t _data;
    _data.ptr = data;

    uint32_t event = 0;
    event |= EPOLLET;
    event |= EPOLLIN;
    WBaseEpoll::AddSocket(data->_timerfd, event, _data);
}

void WTimerEpoll::RemoveTimer(WTimerHandlerData* data)
{
    WBaseEpoll::RemoveSocket(data->_timerfd);
}


} // namespace wlb::NetWork

#endif // OS_IS_LINUX
