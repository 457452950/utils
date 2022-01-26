#pragma once

#include "WEpoll.hpp"
#include "WBuffer.hpp"
#include "WNetWorkHandler.hpp"

namespace wlb::NetWork
{



/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Accepter
////////////////////////////////////////////////////////

class WNetAccepter
{
public:
    explicit WNetAccepter() = default;
    WNetAccepter(const WNetAccepter& other) = delete;
    WNetAccepter& operator=(const WNetAccepter& other) = delete;

    bool Init(WNetWorkHandler* handler, const std::string& IpAddress, uint16_t port);
    base_socket_type GetListenSocket();
    
    base_socket_type Accept();

private:
    base_socket_type _socket;

    WNetWorkHandler* _handler;
};





/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Seession
////////////////////////////////////////////////////////

class WBaseSession : public WNetWorkHandler::Listener
{
public:
    virtual ~WBaseSession() {};

    // class life control
    virtual bool Init(WNetWorkHandler* handler, uint32_t maxBufferSize, uint32_t flags = 0) = 0;
    virtual bool SetSocket(base_socket_type socket) = 0;
    virtual void Clear() = 0;
    virtual void Destroy() = 0;

    // session methods
    virtual bool Send(const std::string& message) = 0;
    virtual bool Receive() = 0;
    virtual void Close() = 0;
    virtual void HandleError() = 0;

    // // get session state
    // virtual bool isConnected() = 0;
    // virtual const std::string& getErrorMessage() = 0;

    // virtual const std::string& getPeerIpAddress() = 0;
    // virtual const std::string& getPeerPort() = 0;

// protected:  // unablecopy
//     WBaseSession(const WBaseSession& other) = delete;
//     WBaseSession& operator=(const WBaseSession& other) = delete;
};

struct wlbHead2
{
    union _head
    {
        uint16_t length;
        uint8_t data[2];
    }data;
    wlbHead2(uint16_t len = 0) {data.length = len;};
};
struct wlbHead4
{
    union _head
    {
        uint32_t length;
        uint8_t data[4];
    }data;
    wlbHead4(uint32_t len = 0) {data.length = len;};
};
const std::string MakeWlbHead(const wlbHead2& length);
const std::string MakeWlbHead(const wlbHead4& length); 
uint32_t GetLengthFromWlbHead(const char* wlbHead, uint32_t head_length);



class WFloatBufferSession : public WBaseSession
{
    // wlb Head = 4
    //   0 1 2 3 4 5 6 7 8 9
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |s|i|z|e|  ... message body  ...|    

public:
    explicit WFloatBufferSession() = default;
    WFloatBufferSession(const WBaseSession& other) = delete;
    WFloatBufferSession& operator=(const WBaseSession& other) = delete;
    virtual ~WFloatBufferSession() = default;

    // class life time
    bool Init(WNetWorkHandler* handler, uint32_t maxBufferSize, uint32_t headLen = 4) override;
    inline bool SetSocket(base_socket_type socket) override;
    void Clear() override;
    void Destroy() override;

    // class methods
    bool Send(const std::string& message) override;
    bool Receive() override;
    void Close() override;
    void HandleError() override;

    // get session state
    // bool isConnected() override;
    // const std::string& getErrorMessage() override;

    // const std::string& getPeerIpAddress() override;
    // const std::string& getPeerPort() override;

public:
    // return false if need to close
    bool OnError(base_socket_type socket, std::string error) override;
    bool OnClosed(base_socket_type socket) override;
    bool OnRead(base_socket_type socket) override;
    bool OnWrite(base_socket_type socket) override;

    
private:
    RingBuffer _recvBuffer;
    RingBuffer _sendBuffer;

    base_socket_type _socket;
    uint32_t _op;

    uint32_t _maxBufferSize;
    uint32_t _headLen;

    // from out side
    WNetWorkHandler* _handler;
};

/**
class WFixedBufferSession : public WBaseSession, public RingBuffer
{
public:
    WFixedBufferSession() = default;
    WFixedBufferSession(const WFixedBufferSession& other) = delete;
    WFixedBufferSession& operator=(const WFixedBufferSession& other) = delete;

    // class life time
    bool Init(WNetWorkHandler* handler, uint32_t maxBufferSize, uint32_t headSize) override;
    void Close() override;
    void Destroy() override;

    // class methods
    bool Send(const std::string& message) override;
    bool Receive() override;
    void Close() override;
    void HandleError() override;

    // get session state
    bool isConnected() override;
    const std::string& getErrorMessage() override;

    const std::string& getPeerIpAddress() override;
    const std::string& getPeerPort() override;
};
*/


}   // namespace wlb::NetWork
