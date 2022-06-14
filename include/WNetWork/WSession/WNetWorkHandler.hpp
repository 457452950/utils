#pragma once

#include <cstdint>
#include "../../WOS.h"
#include "../WNetWorkUtils.h"

namespace wlb::network {

struct WHandlerData;

class WNetWorkHandler {
public:
    class Listener {
    public:
        virtual ~Listener() = default;

    public:
        virtual void OnError(int16_t error_code) = 0;
        virtual void OnClosed()                  = 0;
        virtual void OnShutdown()                = 0;
        virtual void OnRead()                    = 0;
        virtual void OnWrite()                   = 0;
    };

    using ListenOptions = uint32_t;
    enum OP {
        OP_IN   = 0x1 << 0,
        OP_OUT  = 0x1 << 1,
        OP_ERR  = 0x1 << 2,
        OP_SHUT = 0x1 << 3,
        OP_CLOS = 0x1 << 4,
    };

public:
    WNetWorkHandler()          = default;
    virtual ~WNetWorkHandler() = default;
    // no copyable
    WNetWorkHandler(const WNetWorkHandler &other)       = delete;
    WNetWorkHandler &operator=(const WNetWorkHandler &) = delete;

    virtual bool Init(uint32_t events_size)        = 0;
    virtual void Close()                           = 0;
    virtual void GetAndEmitEvents(int32_t timeout) = 0;

    // control
    virtual bool AddSocket(WHandlerData *data, uint32_t op_event)    = 0;
    virtual bool ModifySocket(WHandlerData *data, uint32_t op_event) = 0;
    virtual void RemoveSocket(base_socket_type socket)               = 0;

    virtual int16_t GetErrorNo() = 0;
};

// 注册登记时存放的结构
struct WHandlerData {
    WNetWorkHandler::Listener *listener;
    base_socket_type          &socket;
    WHandlerData(base_socket_type &socket, WNetWorkHandler::Listener *listener) : listener(listener), socket(socket) {}
};

// User define its self
WNetWorkHandler *CreateNetworkHandlerAndInit(uint32_t events_size);

} // namespace wlb::network
