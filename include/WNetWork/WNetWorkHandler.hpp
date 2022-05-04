#pragma once

#include "../WOS.h"
#include <stdint.h>
#include "WNetWorkUtils.h"

namespace wlb::NetWork
{

struct WHandlerData;

class WNetWorkHandler
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;
    public:
        virtual void OnError(int error_code) = 0;
        virtual void OnClosed() = 0;
        // virtual void OnConnected() = 0;
        virtual void OnShutdown() = 0;
        virtual void OnRead() = 0;
        virtual void OnWrite() = 0;
    };

    using ListenOptions = uint32_t;
    enum OP{
        OP_IN = 0x1 << 0,
        OP_OUT = 0x1 << 1,
        OP_ERR = 0x1 << 2,
        OP_SHUT = 0x1 << 3,
        OP_CLOS = 0x1 << 4,
    };

public:
    WNetWorkHandler() = default;
    virtual ~WNetWorkHandler() = default;
    WNetWorkHandler(const WNetWorkHandler& other) = delete;
    WNetWorkHandler& operator=(const WNetWorkHandler&) = delete;

    virtual bool Init(uint32_t events_size) = 0;
    virtual void Close() = 0;
    virtual void GetAndEmitEvents(int32_t timeout = 0) = 0;

    virtual bool AddSocket(WHandlerData* data, uint32_t op_event) = 0;
    virtual bool ModifySocket(WHandlerData* data, uint32_t op_event) = 0;
    virtual void RemoveSocket(base_socket_type socket) = 0;

    virtual std::string GetErrorMessage() = 0;
};



struct WHandlerData
{
    WNetWorkHandler::Listener *listener;
    base_socket_type& socket;
    WHandlerData(base_socket_type& socket, WNetWorkHandler::Listener *listener)
        : listener(listener), socket(socket) {}
};

WNetWorkHandler* CreateNetworkHandlerAndInit(uint32_t events_size);

}

