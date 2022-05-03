#pragma once

#include "WEpoll.hpp"
#include "../WBuffer.hpp"
#include "WNetWorkHandler.hpp"
#include "WService.hpp"
#include "../WDebugger.hpp"

namespace wlb::NetWork
{

using namespace wlb::debug;


/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Accepter
////////////////////////////////////////////////////////

class WNetAccepter : public WNetWorkHandler::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {};
        virtual bool OnNewConnection(base_socket_type socket, const WEndPointInfo& peerInfo) = 0;
    };
public:
    explicit WNetAccepter(Listener* listener);
    WNetAccepter(const WNetAccepter& other) = delete;
    WNetAccepter& operator=(const WNetAccepter& other) = delete;
    ~WNetAccepter();

    bool Init(WNetWorkHandler* handler, const std::string& IpAddress, uint16_t port);
    void Close();
    base_socket_type GetListenSocket();
    
protected:
    base_socket_type Accept(WEndPointInfo& info);

protected:
    void OnError(int error_code) override;
    void OnRead() override;

    // not uese
    void OnClosed() override {};
    void OnShutdown() override {};
    void OnWrite() override {};
    // virtual bool OnConnected() override{};


private:
    bool Listen();

private:
    base_socket_type _socket;
    // bind address ip and port
    std::string _address;
    uint16_t _port;

    WNetWorkHandler* _handler{nullptr};
    Listener* _listener{nullptr};

    WHandlerData* _handlerData{nullptr};
};





/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Seession
////////////////////////////////////////////////////////


// 网络层抽象逻辑
class WBaseConnection
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {};
        virtual void OnConnectionMessage(const std::string& recieve_message) = 0;
        virtual void OnConnectionClosed() = 0;
        virtual void OnConnectionShutdown() = 0;
        virtual void OnConnectionError() = 0;
    };
public:
    WBaseConnection() {};
    virtual ~WBaseConnection() {};

    // class life control
    virtual bool Init(WNetWorkHandler* handler, uint32_t maxBufferSize, uint32_t flags = 0) = 0;
    virtual bool SetConnectedSocket(base_socket_type socket,const WEndPointInfo& peerInfo) = 0;
    virtual void Clear() = 0;
    virtual void Destroy() = 0;

    // connection methods
    virtual bool Send(const std::string& message) = 0;
    virtual bool Receive() = 0;
    virtual void Close() = 0;   // close the connection not class

    // get connection state
    virtual bool isConnected() = 0;
    // virtual int GetErrorCode() = 0;
    virtual const std::string& getErrorMessage() = 0;

    virtual const std::string& getPeerIpAddress() = 0;
    virtual const uint16_t getPeerPort() = 0;
    
protected:
    std::string _errorMessage;
};


//////////////////////////////////
// there are two classes of WBaseConnection 


enum class WConnectionType {
    WFloatConnections = 0,
    WFixedConnections = 1,
};

struct WConnectionStyle {
    WConnectionType type;
    uint32_t maxBufferSize;
    uint32_t flag;
    WConnectionStyle(WConnectionType type = WConnectionType::WFixedConnections, uint32_t maxBufferSize = 0, uint32_t flag = 0)
        : type(type), maxBufferSize(maxBufferSize), flag(flag) { };
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



class WFloatBufferConnection : public WBaseConnection, public WNetWorkHandler::Listener
{
    //  Head = 4  message length = 0x1234
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |0|1|2|3|4|5|6|7|8|9|...
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |4|3|2|1|  ... message body  ...|    
public:
    explicit WFloatBufferConnection(WBaseConnection::Listener* listener);
    WFloatBufferConnection(const WFloatBufferConnection& other) = delete;
    WFloatBufferConnection& operator=(const WFloatBufferConnection& other) = delete;
    virtual ~WFloatBufferConnection();

    // class life time
    bool Init(WNetWorkHandler* handler, uint32_t maxBufferSize, uint32_t headLen = 4) override;
    bool SetConnectedSocket(base_socket_type socket, const WEndPointInfo& peerInfo) override;
    void Clear() override;      // 重置类
    void Destroy() override;

    // class methods
    bool Send(const std::string& message) override;
    bool Receive() override;
    
    void Close() override;  // 关闭连接 close the connection not socket

    // get connection state
    inline bool isConnected() override { return this->_isConnected; };
    inline const std::string& getErrorMessage() override { return this->_errorMessage; };

    const std::string& getPeerIpAddress() override { return this->_peerInfo.ip_address; };
    const uint16_t getPeerPort() override { return this->_peerInfo.port; };

private:
    void HandleError(int error_code);  // 错误处理

protected:
    // return false if need to close
    void OnError(int error_code) override;
    void OnClosed() override;
    void OnRead() override;
    void OnWrite() override;
    void OnShutdown() override;

    
private:
    // connection members
    RingBuffer _recvBuffer;
    RingBuffer _sendBuffer;

    base_socket_type _socket;
    uint32_t _op{0};

    uint32_t _maxBufferSize{0};
    uint32_t _headLen{0};

    // connection state
    bool _isConnected{false};
    std::string _errorMessage;

    // peer information
    WEndPointInfo _peerInfo;

    // from out side
    WNetWorkHandler* _handler{nullptr};
    WBaseConnection::Listener* _listener{nullptr};

    WHandlerData* _handlerData{nullptr};
};

class WFixedBufferConnection : public WBaseConnection, public WNetWorkHandler::Listener
{
public:
    explicit WFixedBufferConnection(WBaseConnection::Listener* listener);
    WFixedBufferConnection(const WFixedBufferConnection& other) = delete;
    WFixedBufferConnection& operator=(const WFixedBufferConnection& other) = delete;
    virtual ~WFixedBufferConnection();

    // class life time
    bool Init(WNetWorkHandler* handler, uint32_t maxBufferSize, uint32_t messageSize = 0) override;
    bool SetConnectedSocket(base_socket_type socket, const WEndPointInfo& peerInfo) override;
    void Clear() override;
    void Destroy() override;

    // class methods
    bool Send(const std::string& message) override;
    bool Receive() override;
    // close 
    void Close() override;  // close the connection not socket

    // get connection state
    inline bool isConnected() override { return this->_isConnected; };
    inline const std::string& getErrorMessage() override { return this->_errorMessage; };

    const std::string& getPeerIpAddress() override { return this->_peerInfo.ip_address; };
    const uint16_t getPeerPort() override { return this->_peerInfo.port; };

private:
    void HandleError(int error_code);

protected:
    // return false if need to close
    void OnError(int error_code) override;
    void OnClosed() override;
    void OnRead() override;
    void OnWrite() override;
    void OnShutdown() override;

    
private:
    // connection members
    RingBuffer _recvBuffer;
    RingBuffer _sendBuffer;

    base_socket_type _socket;
    uint32_t _op{0};

    uint32_t _maxBufferSize{0};
    uint32_t _messageSize{0};

    // connection state
    bool _isConnected{false};
    std::string _errorMessage;

    // peer information
    WEndPointInfo _peerInfo;

    // from out side
    WNetWorkHandler* _handler{nullptr};
    WBaseConnection::Listener* _listener{nullptr};

    WHandlerData* _handlerData{nullptr};
};



}   // namespace wlb::NetWork
