#include "WNetWork/WConnection.hpp"
#include <iostream>
#include "../WDebugger.hpp"

using namespace wlb::debug;

#define ACCEPTERADD \
DEBUGADD("WNetAccepter")
#define ACCEPTERRM \
DEBUGRM("WNetAccepter")

#define FLOATSESSADD \
DEBUGADD("WFloatBufferConnection")
#define FLOATSESSRM \
DEBUGRM("WFloatBufferConnection")

#define FIXEDSESSADD \
DEBUGADD("WFixedBufferConnection")
#define FIXEDSESSRM \
DEBUGRM("WFixedBufferConnection")

namespace wlb::NetWork
{

/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Accepter
////////////////////////////////////////////////////////


WNetAccepter::WNetAccepter(Listener* listener) : _listener(listener) 
{
    ACCEPTERADD;
}

WNetAccepter::~WNetAccepter() 
{
    ACCEPTERRM;
    this->Close();
}

bool WNetAccepter::Init(WNetWorkHandler* handler, const std::string& IpAddress, uint16_t port)
{
    /////////////////////////////////
    // Initialize socket
    this->_socket = MakeTcpv4Socket();       // tcp v4
    if (this->_socket == -1)
    {
        return false;
    }
    

    if ( !SetSocketReuseAddr(this->_socket) || 
            !SetSocketReusePort(this->_socket) || 
            !SetSocketKeepAlive(this->_socket) )
    {
        
        return false;
    }
    
    if ( !SetSocketNoBlock(this->_socket) )
    {
        
    }
    

    // ///////////////////////////////////////
    // Init members
    this->_address = IpAddress; // copy
    this->_port = port;
    
    this->_handler = handler;
    if (this->_handler == nullptr)
    {
        
        return false;
    }
    

    // ///////////////////////////////////////
    // Bind 
    if ( !wlb::NetWork::Bind(this->_socket, this->_address, this->_port) )
    {
        return false;
    }
    

    /////////////////////////////////
    // Listen
    if ( !this->Listen() )
    {
        
        return false;
    }
    
    this->_handlerData = new(std::nothrow) WHandlerData(this->_socket, this);
    if (this->_handlerData == nullptr)
    {
        return false;
    }
    
    // //////////////////////////////
    // add socket in handler
    uint32_t op = 0;
    op |= WNetWorkHandler::OP_IN;
    op |= WNetWorkHandler::OP_ERR;
    
    if ( !this->_handler->AddSocket(this->_handlerData, op) )
    {
        
        return false;
    }
    
    
    return true;
}


bool WNetAccepter::Listen()
{
    if ( ::listen(this->_socket, 1024) == -1) 
    {
        return false;
    }
    return true;
}

base_socket_type WNetAccepter::Accept(WEndPointInfo& info)
{
    return wlb::NetWork::Accept(this->_socket, info, true);
}

base_socket_type WNetAccepter::GetListenSocket()
{
    return this->_socket;
}

void WNetAccepter::Close()
{
    this->_handler->RemoveSocket(this->_socket);
    ::close(this->_socket);
    this->_handler = nullptr;
    this->_listener = nullptr;

    if (this->_handlerData != nullptr)
    {
        delete this->_handlerData;
    }
}

void WNetAccepter::OnRead()
{
    WEndPointInfo info;
    base_socket_type cli_sock = this->Accept(info);

    if (this->_listener != nullptr)
    {
        if (!this->_listener->OnNewConnection(cli_sock, info))
        {
            ::close(cli_sock);
            std::cout << "close client" << info.ip_address << " " << info.port << std::endl;
        }
    }
    
    return;
}

void WNetAccepter::OnError(int error_code)
{
    this->Close();
    return;
}






/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Seession
////////////////////////////////////////////////////////

WFloatBufferConnection::WFloatBufferConnection(WBaseConnection::Listener* listener):
    _listener(listener) 
{
    FLOATSESSADD;
}
WFloatBufferConnection::~WFloatBufferConnection()
{
    FLOATSESSRM;
    this->Destroy();
}

bool WFloatBufferConnection::Init(WNetWorkHandler* handler, uint32_t maxBufferSize, uint32_t headLen)
{
    this->_maxBufferSize = maxBufferSize;
    this->_headLen = headLen;

    if ( !_recvBuffer.Init(maxBufferSize) )
    {
        this->_errorMessage = this->_recvBuffer.GetErrorMessage();
        return false;
    }
    if ( !_sendBuffer.Init(maxBufferSize) )
    {
        this->_errorMessage = this->_sendBuffer.GetErrorMessage();
        return false;
    }

    this->_handler = handler;
    if (this->_handler == nullptr)
    {
        this->_errorMessage = "handler is nullptr";
        return false;
    }
    
    this->_op |= WNetWorkHandler::OP_IN;
    this->_op |= WNetWorkHandler::OP_ERR;
    this->_op |= WNetWorkHandler::OP_SHUT;
    this->_op |= WNetWorkHandler::OP_CLOS;

    this->_handlerData = new(std::nothrow) WHandlerData(this->_socket, this);
    if (this->_handlerData == nullptr)
    {
        return false;
    }

    return true;
}

bool WFloatBufferConnection::SetConnectedSocket(base_socket_type socket, const WEndPointInfo& peerInfo)
{
    this->_socket = socket;

    if (    !SetTcpSocketNoDelay(this->_socket) || 
            !SetSocketKeepAlive(this->_socket) )
    {
        this->_errorMessage = strerror(errno);
        return false;
    }
    if ( !SetSocketNoBlock(this->_socket))
    {
        this->_errorMessage = strerror(errno);
    }
    if ( !_handler->AddSocket(this->_handlerData, this->_op))
    {
        this->_errorMessage = this->_handler->GetErrorMessage();
        return false;
    }

    this->_peerInfo = peerInfo;

    this->_isConnected = true;

    return true;
}

void WFloatBufferConnection::Close()
{
    if (::shutdown(this->_socket, SHUT_RDWR) == -1)
    {
        this->_errorMessage = strerror(errno);
    }
}

void WFloatBufferConnection::Clear()
{
    this->_handler->RemoveSocket(this->_socket);
    this->_isConnected = false;
    ::close(this->_socket);
    this->_socket = -1;
    _recvBuffer.Clear();
    _sendBuffer.Clear();
}

void WFloatBufferConnection::Destroy()
{
    _recvBuffer.Destroy();
    _sendBuffer.Destroy();
    this->_listener = nullptr;
    if (this->_handlerData != nullptr)
    {
        delete this->_handlerData;
    }
    
}

bool WFloatBufferConnection::Send(const std::string& message)
{
    uint32_t msgSize = message.size();
    uint32_t insert_len = 0;

    std::string head;
    if (this->_headLen == 2)
    {
        head = MakeWlbHead(wlbHead2(msgSize));
    }
    else if (this->_headLen == 4)
    {
        head = MakeWlbHead(wlbHead4(msgSize));
    }
    
    std::string send_message = head + message;
    insert_len = _sendBuffer.InsertMessage(send_message);
    if (insert_len != send_message.length())
    {
        this->_errorMessage = "insert message false";
        return false;
    }

    std::cout << "send message start set epoll out" << std::endl;
    if (!(this->_op & WNetWorkHandler::OP_OUT))
    {
        std::cout << "send message set epoll out" << std::endl;
        // 添加进 send events
        this->_op |= WNetWorkHandler::OP_OUT;
        if ( !_handler->ModifySocket(this->_handlerData, this->_op) )
        {
            std::cout << "send message set epoll out failed" << std::endl;
            this->_errorMessage = this->_handler->GetErrorMessage();
            std::cout << "send message set epoll out failed" << this->_errorMessage << std::endl;
            return false;
        }
    }    
    std::cout << "WFloatBufferConnection::Send end" << std::endl;
    return true;
}

bool WFloatBufferConnection::Receive()
{
    std::cout << "WFloatBufferConnection::Receive" << std::endl;
    int32_t recv_len = ::recv(this->_socket, 
                                this->_recvBuffer.GetRestBuffer(), 
                                this->_recvBuffer.GetTopRestBufferSize(),
                                0);
    if (recv_len <= -1)
    {
        this->HandleError(errno);
        std::cout << this->_errorMessage << std::endl;
        this->_listener->OnConnectionError();
        return false;
    }
    if (recv_len == 0 && _recvBuffer.GetTopRestBufferSize() != 0)
    {
        this->_errorMessage = "recv 0";
        std::cout << this->_errorMessage << std::endl;
        return false;
    }
    
    this->_recvBuffer.UpdateWriteOffset(recv_len);

    return true;
}

void WFloatBufferConnection::HandleError(int error_code)
{
    this->_errorMessage.clear();
    this->_errorMessage.assign(::strerror(error_code));
}

void WFloatBufferConnection::OnError(int error_no)
{
    HandleError(error_no);

    if (this->_listener != nullptr)
    {
        this->_listener->OnConnectionError();
    }
    
    return;
}

void WFloatBufferConnection::OnClosed()
{   
    if (this->_listener != nullptr)
    {
        this->_listener->OnConnectionClosed();
    }
    return;
}

void WFloatBufferConnection::OnRead()
{
    if (!this->Receive())
    {
        ::shutdown(this->_socket, SHUT_RD);
        return;
    }
    std::cout << "WFloatBufferConnection::OnRead" << std::endl;
    
    std::string head;
    std::string receive_message;
    // on message
    while (1)
    {
        receive_message.clear();
        uint32_t len = this->_recvBuffer.GetFrontMessage(head, this->_headLen);
        
        if (len != this->_headLen)
        {
            std::cout << "WFloatBufferConnection::OnRead no enough message len:" << len << std::endl;
            break;
        }
        
        len = GetLengthFromWlbHead(head.c_str(), this->_headLen);
        std::cout << "WFloatBufferConnection::OnRead head body length:" << len << std::endl;
        
        if (len == 0)
        {
            this->_errorMessage = "Invalid message head len = 0";
            this->_listener->OnConnectionError();
            std::cout << this->_errorMessage << std::endl;
            return;
        }

        if (len > this->_recvBuffer.GetFrontMessageLength())
        {
            std::cout << "WFloatBufferConnection::OnRead no enough message len" << len << std::endl;
            return;
        }
        
        this->_recvBuffer.UpdateReadOffset(this->_headLen);

        len = this->_recvBuffer.GetFrontMessage(receive_message, len);
        if (len == 0)
        {
            // no enough message
            std::cout << "WFloatBufferConnection::OnRead error no enough message len" << len << std::endl;
            break;
        }

        (len > 0) ? this->_recvBuffer.UpdateReadOffset(len)
                    : this->_recvBuffer.UpdateReadOffset(0);
        
        this->_listener->OnConnectionMessage(receive_message);
    }
}

void WFloatBufferConnection::OnWrite()
{
    std::cout << "WFloatBufferConnection::OnWrite()" << std::endl;
    std::string send_message;
    uint32_t msg_len = _sendBuffer.GetAllMessage(send_message);
    ssize_t send_len = ::send(this->_socket, send_message.c_str(), msg_len, 0);
    
    std::cout << "sned len :" << send_len << std::endl;

    this->_sendBuffer.UpdateReadOffset(send_len);

    if (send_len < 0)
    {
        this->HandleError(errno);
        this->_listener->OnConnectionError();
        return;
    }

    if (_sendBuffer.Empty())
    {
        this->_op -= WNetWorkHandler::OP_OUT;
        this->_handler->ModifySocket(this->_handlerData, this->_op);
    }

    return;
}

void WFloatBufferConnection::OnShutdown()
{
    if (this->_listener != nullptr)
    {
        this->_listener->OnConnectionShutdown();
    }
    
    return;
}








// wlb style methods

const std::string MakeWlbHead(const wlbHead2& length)
{
    std::string head = "";
    head.append((char*)&length.data.data, 2);
    return head;
}
const std::string MakeWlbHead(const wlbHead4& length)
{
    std::string head = "";
    head.append((char*)&length.data.data, 4);
    return head;
}

uint32_t GetLengthFromWlbHead(const char* wlbHead, uint32_t head_length)
{
    if (head_length == 0)
    {
        return 0;
    }
    else if (head_length == 2)
    {
        wlbHead2 head;
        try
        {
            memcpy(head.data.data, wlbHead, 2);
            return head.data.length;
        }
        catch(const std::exception& e)
        {
            // error
            // std::cout << e.what() << '\n';
            return 0;
        }
    }
    else if (head_length == 4)
    {
        wlbHead4 head;
        try
        {
            memcpy(head.data.data, wlbHead, 4);
            return head.data.length;
        }
        catch(const std::exception& e)
        {
            // error
            // std::cout << e.what() << '\n';
            return 0;
        }
    }
    return 0;
}







/////////////////////////////////
// WFixedBufferConnection


WFixedBufferConnection::WFixedBufferConnection(WBaseConnection::Listener* listener):
    _listener(listener) 
{
    FIXEDSESSADD;
}
WFixedBufferConnection::~WFixedBufferConnection()
{
    FIXEDSESSRM;
}

bool WFixedBufferConnection::Init(WNetWorkHandler* handler, uint32_t maxBufferSize, uint32_t messageSize)
{
    this->_maxBufferSize = maxBufferSize;
    this->_messageSize = messageSize;

    if ( !_recvBuffer.Init(maxBufferSize) )
    {
        this->_errorMessage = this->_recvBuffer.GetErrorMessage();
        return false;
    }
    if ( !_sendBuffer.Init(maxBufferSize) )
    {
        this->_errorMessage = this->_sendBuffer.GetErrorMessage();
        return false;
    }

    this->_handler = handler;
    if (this->_handler == nullptr)
    {
        this->_errorMessage = "Invalid _handler nullptr";
        return false;
    }
    
    this->_op |= WNetWorkHandler::OP_IN;
    this->_op |= WNetWorkHandler::OP_ERR;
    this->_op |= WNetWorkHandler::OP_SHUT;
    this->_op |= WNetWorkHandler::OP_CLOS;


    this->_handlerData = new(std::nothrow) WHandlerData(this->_socket, this);
    if (this->_handlerData == nullptr)
    {
        return false;
    }
    

    return true;
}

bool WFixedBufferConnection::SetConnectedSocket(base_socket_type socket, const WEndPointInfo& peerInfo)
{
    this->_socket = socket;

    if (    !SetTcpSocketNoDelay(this->_socket) || 
            !SetSocketKeepAlive(this->_socket) )
    {
        
        return false;
    }
    if ( !SetSocketNoBlock(this->_socket))
    {
        // error set noblock failed
    }
    if ( !_handler->AddSocket(this->_handlerData, this->_op))
    {
        return false;
    }

    this->_peerInfo = peerInfo;

    this->_isConnected = true;

    return true;
}

void WFixedBufferConnection::Close()
{
    shutdown(this->_socket, SHUT_RDWR);
}

void WFixedBufferConnection::Clear()
{
    this->_handler->RemoveSocket(this->_socket);
    this->_isConnected = false;
    ::close(this->_socket);
    this->_socket = -1;

    _recvBuffer.Clear();
    _sendBuffer.Clear();
}

void WFixedBufferConnection::Destroy()
{
    _recvBuffer.Destroy();
    _sendBuffer.Destroy();
    this->_listener = nullptr;
    if (this->_handlerData != nullptr)
    {
        delete this->_handlerData;
    }
}

bool WFixedBufferConnection::Send(const std::string& message)
{
    uint32_t msgSize = message.size();
    uint32_t insert_len = 0;

    if (msgSize > this->_messageSize)
    {
        // message is too large
        return false;
    }
    
    insert_len = _sendBuffer.InsertMessage(message);
    if (insert_len != message.length())
    {
        return false;
    }

    // 添加进 send events
    this->_op |= WNetWorkHandler::OP_OUT;
    if ( !_handler->ModifySocket(this->_handlerData, this->_op) )
    {
        return false;
    }
    
    return true;
}

bool WFixedBufferConnection::Receive()
{
    int32_t recv_len = ::recv(this->_socket, 
                                this->_recvBuffer.GetRestBuffer(), 
                                this->_recvBuffer.GetTopRestBufferSize(),
                                0);
    if (recv_len <= -1)
    {
        
        return false;
    }
    if (recv_len == 0 && _recvBuffer.GetTopRestBufferSize() != 0)
    {
        
        return false;
    }
    
    this->_recvBuffer.UpdateWriteOffset(recv_len);

    return true;
}

void WFixedBufferConnection::HandleError(int error_code)
{
    this->_errorMessage.clear();
    this->_errorMessage.assign(::strerror(error_code));

}

void WFixedBufferConnection::OnError(int error_no)
{
    HandleError(error_no);
    
    if (this->_listener != nullptr)
    {
        this->_listener->OnConnectionError();
    }
    
    return;
}

void WFixedBufferConnection::OnClosed()
{
    if (this->_listener != nullptr)
    {
        this->_listener->OnConnectionClosed();
    }
    return;
}

void WFixedBufferConnection::OnRead()
{
    if (!this->Receive())
    {
        ::shutdown(this->_socket, SHUT_RD);
        return;
    }
    
    std::string receive_message;
    // on message
    while (1)
    {
        receive_message.clear();

        uint32_t len = this->_recvBuffer.GetFrontMessage(receive_message, this->_messageSize);
        
        if (len != this->_messageSize)
        {
            
            return;
        }
        

        this->_listener->OnConnectionMessage(receive_message);
    }
}

void WFixedBufferConnection::OnWrite()
{
    std::string send_message;
    uint32_t msg_len = _sendBuffer.GetAllMessage(send_message);
    ssize_t send_len = ::send(this->_socket, send_message.c_str(), msg_len, 0);
    // std::cout << "send:" << send_len << std::endl;
    
    this->_sendBuffer.UpdateReadOffset(send_len);

    if (send_len < 0)
    {
        this->HandleError(errno);
        this->_listener->OnConnectionError();
        return;
    }

    if (_sendBuffer.Empty())
    {
        this->_op -= WNetWorkHandler::OP_OUT;
        this->_handler->ModifySocket(this->_handlerData, this->_op);
    }

    return;
}

void WFixedBufferConnection::OnShutdown()
{
    if (this->_listener != nullptr)
    {
        this->_listener->OnConnectionShutdown();
    }
    
    if ( ::shutdown(this->_socket, SHUT_RDWR) == -1 )
    {
        
        return;
    }
    return;
}



}

