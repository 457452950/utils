#pragma once
#include "WOS.h"
#include <stdint.h>
#include "WNetWorkUtils.h"

namespace wlb::NetWork
{

class WNetWorkHandler
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {};
        
    public:
        virtual bool OnError(base_socket_type socket, std::string error) = 0;
        virtual bool OnClosed(base_socket_type socket) = 0;
        // virtual bool OnConnected(base_socket_type socket) = 0;
        virtual bool OnShutdown(base_socket_type socket) = 0;
        virtual bool OnRead(base_socket_type socket) = 0;
        virtual bool OnWrite(base_socket_type socket) = 0;
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
    WNetWorkHandler(const WNetWorkHandler& other) = delete;
    WNetWorkHandler& operator=(const WNetWorkHandler&) = delete;

    virtual bool Init(uint32_t events_size) = 0;
    virtual void Close()  = 0;
    virtual void GetAndEmitEvents() = 0;

    virtual bool AddSocket(base_socket_type socket, uint32_t events) = 0;
    virtual bool ModifySocket(base_socket_type socket, uint32_t events) = 0;
    virtual void RemoveSocket(base_socket_type socket) = 0;

};










}

