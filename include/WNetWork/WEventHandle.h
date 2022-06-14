#pragma once

#include <forward_list>

#include "WEpoll.h"
#include "WEvent.h"
#include "WSelect.h"


namespace wlb::network {

class WEventHandle {
public:
    virtual ~WEventHandle(){};

    class Listener {
        virtual void OnEvent(uint8_t event_type) = 0;
    };

    class event_data {
    public:
        using user_data_ptr = void *;
        event_data(base_socket_type socket, user_data_ptr user_data) : socket_(socket), user_data_(user_data){};
        ~event_data() {}

        inline base_socket_type GetSocket() { return socket_; }
        inline bool             CacheEventError() { return flag & (1 << 0); }
        inline bool             CacheEventRead() { return flag & (1 << 1); }
        inline bool             CacheEventWrite() { return flag & (1 << 2); }
        inline void             CacheEventError(bool v) { flag |= (v << 0); }
        inline void             CacheEventRead(bool v) { flag |= (v << 1); }
        inline void             CacheEventWrite(bool v) { flag |= (v << 2); }

    private:
        base_socket_type socket_{-1};
        user_data_ptr    user_data_{nullptr};
        int8_t           flag{0};
    };

    using fd_list      = std::forward_list<event_data>;
    using fd_list_item = fd_list::iterator;

    fd_list_item NewEvent(base_socket_type socket, event_data::user_data_ptr user_data);
    void         EraseEvent(fd_list_item item);

private:
    virtual bool AddSocket(base_socket_type socket, event_data::user_data_ptr user_data) { return true; };
    virtual void DelSocket(base_socket_type socket){};
    virtual void ModifySocket(fd_list_item item){};

    virtual void EventLoop() = 0;

protected:
    fd_list list;
};

class WSelect final : public WEventHandle {
public:
    WSelect();
    ~WSelect() override;

    bool AddSocket(base_socket_type socket, event_data::user_data_ptr user_data) override;

private:
    fd_set_type      read_fd_set_;
    fd_set_type      write_fd_set_;
    fd_set_type      error_fd_set_;
    base_socket_type max_fd_{0};
    uint16_t         fd_count_{0};
};

} // namespace wlb::network
