#pragma once

#include "WEpoll.hpp"
#include "../WBuffer.hpp"
#include "WNetWorkHandler.hpp"
#include "WService.hpp"

namespace wlb::NetWork {



/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Accepter
////////////////////////////////////////////////////////

// tcp v4 only
class WNetAccepter final : public WNetWorkHandler::Listener {
public:
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual bool OnNewConnection(base_socket_type socket, const WEndPointInfo &peerInfo) = 0;
    };
public:
    explicit WNetAccepter(Listener *listener);
    ~WNetAccepter() override;
    // no copyable
    WNetAccepter(const WNetAccepter &other) = delete;
    WNetAccepter &operator=(const WNetAccepter &other) = delete;

    // 初始化
    bool Init(WNetWorkHandler *handler, const std::string &IpAddress, uint16_t port);
    bool Init(WNetWorkHandler *handler, const WEndPointInfo &end_point_info);
    void Close();

    const WEndPointInfo &GetLocalInfo() const;
    base_socket_type GetListenSocket() noexcept;
    int16_t GetErrorNo() noexcept;

protected:
    base_socket_type Accept(WEndPointInfo *info);

protected:
    // override
    void OnError(int16_t error_code) override;
    void OnRead() override;

    // not use
    void OnClosed() override {};
    void OnShutdown() override {};
    void OnWrite() override {};

private:
    bool Listen();

private:
    base_socket_type socket_{-1};
    // bind local address ip and port
    WEndPointInfo    local_info_;
private:
    //
    WNetWorkHandler *handler_{nullptr};
    Listener        *listener_{nullptr};
    //
    WHandlerData    *handler_data_{nullptr};
    // error
    int16_t         errno_{-1};
};





/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Seession
////////////////////////////////////////////////////////

// 网络层抽象逻辑
class WBaseConnection {
public:
    class Listener {
    public:
        // 当链接已关闭时，才调用OnConnectionClosed、OnConnectionError、OnConnectionShutdown
        virtual ~Listener() = default;
        virtual void OnConnectionMessage(const std::string &receive_message) = 0;
        virtual void OnConnectionClosed() = 0;
        virtual void OnConnectionShutdown() = 0;
        virtual void OnConnectionError() = 0;
    };
public:
    WBaseConnection() = default;
    virtual ~WBaseConnection() = default;

    // class life control
    virtual bool Init(WNetWorkHandler *handler, uint32_t maxBufferSize, uint32_t flags) = 0;
    virtual void Clear() = 0;

    // connection methods
    virtual bool SetConnectedSocket(base_socket_type socket, const WEndPointInfo &peerInfo) = 0;
    virtual bool Send(const std::string &message) = 0;
    virtual void CloseConnection() = 0;   // close the connection not class

    // get connection state
    virtual bool isConnected() = 0;
    virtual int GetErrorCode() = 0;
    virtual const WEndPointInfo &GetPeerInfo() = 0;
protected:
    virtual bool Receive() = 0;
};


//////////////////////////////////
// there are two classes of WBaseConnection

//enum class WConnectionType {
//    WFloatConnections = 0,
//    WFixedConnections = 1,
//};
//
//struct WConnectionStyle {
//    WConnectionType type;
//    uint32_t        maxBufferSize;
//    uint32_t        flag;
//    WConnectionStyle(WConnectionType type = WConnectionType::WFixedConnections,
//                     uint32_t maxBufferSize = 0,
//                     uint32_t flag = 0)
//            : type(type), maxBufferSize(maxBufferSize), flag(flag) {};
//};

template <uint8_t LEN>
union wlbHead {
    uint64_t data_int;
    uint8_t  data_uchar[LEN];
};

uint64_t GetLengthFromWlbHead(const char *str_data, uint8_t head_length);

class WFloatBufferConnection : public WBaseConnection, public WNetWorkHandler::Listener {
    //  Head = 4  message length = 0x1234
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |0|1|2|3|4|5|6|7|8|9|...
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |4|3|2|1|  ... message body  ...|    
public:
    explicit WFloatBufferConnection(WBaseConnection::Listener *listener);
    ~WFloatBufferConnection() override;
    // no copyable
    WFloatBufferConnection(const WFloatBufferConnection &other) = delete;
    WFloatBufferConnection &operator=(const WFloatBufferConnection &other) = delete;

    // class life time
    bool Init(WNetWorkHandler *handler, uint32_t maxBufferSize, uint32_t headLen) override;
    // 清理，准备服用
    void Clear() override;
    // 销毁，不再使用
    void Destroy();

    // class methods
    bool SetConnectedSocket(base_socket_type socket, const WEndPointInfo &peerInfo) override;
    bool Send(const std::string &message) override;
    bool Receive() override;
    void CloseConnection() override;  // 关闭连接 close the connection not socket

    // get connection state
    inline bool isConnected() override { return this->is_connected_; };
    int GetErrorCode() override;
    const WEndPointInfo &GetPeerInfo() override;

private:
    void HandleError(int16_t error_code);  // 错误处理

protected:
    // return false if connection need to close
    void OnError(int16_t error_code) override;
    void OnClosed() override;
    void OnRead() override;
    void OnWrite() override;
    void OnShutdown() override;

private:
    // connection members
    base_socket_type socket_{-1};
    uint32_t         op_{0};
    //
    RingBuffer       recv_buffer_;
    RingBuffer       send_buffer_;
    //
    uint32_t         max_buffer_size_{0};
    uint8_t          head_len_{0};
    // connection state
    bool             is_connected_{false};
    int16_t          error_code_{-1};
    // peer information
    WEndPointInfo    peer_info_;

    //
    WNetWorkHandler           *handler_{nullptr};
    WBaseConnection::Listener *listener_{nullptr};
    //
    WHandlerData              *handler_data_{nullptr};
};

class WFixedBufferConnection : public WBaseConnection, public WNetWorkHandler::Listener {
public:
    explicit WFixedBufferConnection(WBaseConnection::Listener *listener);
    ~WFixedBufferConnection() override;
    // no copyable
    WFixedBufferConnection(const WFixedBufferConnection &other) = delete;
    WFixedBufferConnection &operator=(const WFixedBufferConnection &other) = delete;

    // class life time
    bool Init(WNetWorkHandler *handler, uint32_t maxBufferSize, uint32_t messageSize) override;
    bool SetConnectedSocket(base_socket_type socket, const WEndPointInfo &peerInfo) override;
    void Clear() override;
    void Destroy();

    // class methods
    bool Send(const std::string &message) override;
    bool Receive() override;
    // close 
    void CloseConnection() override;  // close the connection not socket

    // get connection state
    inline bool isConnected() override { return this->is_connected_; };
    int GetErrorCode() override;
    const WEndPointInfo &GetPeerInfo() override;

private:
    void HandleError(int16_t error_code);

protected:
    // return false if need to close
    void OnError(int16_t error_code) override;
    void OnClosed() override;
    void OnRead() override;
    void OnWrite() override;
    void OnShutdown() override;

private:
    // connection members
    base_socket_type          socket_{-1};
    uint32_t                  op_{0};
    //
    uint32_t                  max_buffer_size_{0};
    uint32_t                  message_size_{0};
    RingBuffer                recv_buffer_;
    RingBuffer                send_buffer_;
    // connection state
    bool                      is_connected_{false};
    int16_t                   error_code_{-1};
    // peer information
    WEndPointInfo             peer_info_;
    // from out side
    WNetWorkHandler           *handler_{nullptr};
    WBaseConnection::Listener *listener_{nullptr};
    //
    WHandlerData              *handler_data_{nullptr};
};

}   // namespace wlb::NetWork
