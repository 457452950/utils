#include "WNetWork/WSession.hpp"
#include <iostream>
#include "../WDebugger.hpp"

using namespace wlb::debug;

#define ACCEPTERADD \
DEBUGADD("WNetAccepter")
#define ACCEPTERRM \
DEBUGRM("WNetAccepter")

#define FLOATSESSADD \
DEBUGADD("WFloatBufferSession")
#define FLOATSESSRM \
DEBUGRM("WFloatBufferSession")

#define FIXEDSESSADD \
DEBUGADD("WFixedBufferSession")
#define FIXEDSESSRM \
DEBUGRM("WFixedBufferSession")

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
    this->_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);        // tcp v4
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
    
    
    // //////////////////////////////
    // add socket in handler
    uint32_t op = 0;
    op |= WNetWorkHandler::OP_IN;
    op |= WNetWorkHandler::OP_ERR;
    
    if ( !this->_handler->AddSocket(this, this->_socket, op) )
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
}

void WNetAccepter::OnRead()
{
    WEndPointInfo info;
    base_socket_type cli_sock = this->Accept(info);

    if (this->_listener != nullptr)
    {
        this->_listener->OnConnected(cli_sock, info);
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

WFloatBufferSession::WFloatBufferSession(WBaseSession::Listener* listener):_listener(listener) 
{
    FLOATSESSADD;
}
WFloatBufferSession::~WFloatBufferSession()
{
    FLOATSESSRM;
    this->Destroy();
}

bool WFloatBufferSession::Init(WNetWorkHandler* handler, uint32_t maxBufferSize, uint32_t headLen)
{
    this->_maxBufferSize = maxBufferSize;
    this->_headLen = headLen;

    if ( !_recvBuffer.Init(maxBufferSize) )
    {
        return false;
    }
    if ( !_sendBuffer.Init(maxBufferSize) )
    {
        _recvBuffer.Destroy();
        return false;
    }

    this->_handler = handler;
    if (this->_handler == nullptr)
    {
        _recvBuffer.Destroy();
        _sendBuffer.Destroy();
        return false;
    }
    
    this->_op |= WNetWorkHandler::OP_IN;
    this->_op |= WNetWorkHandler::OP_ERR;
    this->_op |= WNetWorkHandler::OP_SHUT;
    this->_op |= WNetWorkHandler::OP_CLOS;

    return true;
}

bool WFloatBufferSession::SetConnectedSocket(base_socket_type socket, const WEndPointInfo& peerInfo)
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
    if ( !_handler->AddSocket(this, this->_socket, this->_op))
    {
        return false;
    }

    this->_peerInfo = peerInfo;

    this->_isConnected = true;

    return true;
}

void WFloatBufferSession::Close()
{
    // close
    this->_isConnected = false;
    ::close(this->_socket);
    this->_socket = -1;

    // clear
    this->Clear();
}

void WFloatBufferSession::Clear()
{
    this->_isConnected = false;
    _recvBuffer.Clear();
    _sendBuffer.Clear();
}

void WFloatBufferSession::Destroy()
{
    _recvBuffer.Destroy();
    _sendBuffer.Destroy();
    this->_listener = nullptr;
}

bool WFloatBufferSession::Send(const std::string& message)
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
        return false;
    }

    // 添加进 send events
    this->_op |= WNetWorkHandler::OP_OUT;
    if ( !_handler->ModifySocket(this->_socket, this->_op) )
    {
        return false;
    }
    
    return true;
}

bool WFloatBufferSession::Receive()
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

void WFloatBufferSession::HandleError(int error_code)
{
    this->_errorMessage.clear();
    this->_errorMessage.assign(::strerror(error_code));

}

void WFloatBufferSession::OnError(int error_no)
{
    HandleError(error_no);

    if (this->_listener != nullptr)
    {
        bool ok = this->_listener->OnSessionError(this->_socket, error_no);
        
    }
    
    this->Close();
    return;
}

void WFloatBufferSession::OnClosed()
{
    if (this->_listener != nullptr)
    {
        this->_listener->OnSessionClosed(this->_socket);
    }
    this->Close();
    return;
}

void WFloatBufferSession::OnRead()
{
    if (!this->Receive())
    {
        ::shutdown(this->_socket, SHUT_RD);
        return;
    }
    
    std::string head;
    std::string receive_message;
    std::string send_message;
    // on message
    while (1)
    {
        receive_message.clear();
        uint32_t len = this->_recvBuffer.GetFrontMessage(head, this->_headLen);
        
        if (len != this->_headLen)
        {
            // no enough message
            return;
        }
        
        len = GetLengthFromWlbHead(head.c_str(), this->_headLen);
        
        if (len == 0)
        {
            ::shutdown(this->_socket, SHUT_RDWR);
            return;
        }

        len = this->_recvBuffer.GetFrontMessage(receive_message, len);
        if (len == 0)
        {
            // no enough message
            break;
        }

        if (!this->_listener->OnSessionMessage(this->_socket, receive_message, send_message))
        {
            ::shutdown(this->_socket, SHUT_RDWR);
            return;
        }
        if ( !send_message.empty() )
        {
            this->Send(send_message);
        }
    }
}

void WFloatBufferSession::OnWrite()
{
    std::string send_message;
    uint32_t msg_len = _sendBuffer.GetAllMessage(send_message);
    ssize_t send_len = ::send(this->_socket, send_message.c_str(), msg_len, 0);
    
    this->_sendBuffer.UpdateReadOffset(send_len);

    if (send_len < 0)
    {
        ::shutdown(this->_socket, SHUT_RDWR);
        return;
    }

    if (_sendBuffer.Empty())
    {
        this->_op -= WNetWorkHandler::OP_OUT;
        this->_handler->ModifySocket(this->_socket, this->_op);
    }

    return;
}

void WFloatBufferSession::OnShutdown()
{
    if (this->_listener != nullptr)
    {
        this->_listener->OnSessionShutdown(this->_socket);
    }
    
    if ( ::shutdown(this->_socket, SHUT_RDWR) == -1 )
    {
        
        return;
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
// WFixedBufferSession


WFixedBufferSession::WFixedBufferSession(WBaseSession::Listener* listener):_listener(listener) 
{
    FIXEDSESSADD;
}
WFixedBufferSession::~WFixedBufferSession()
{
    FIXEDSESSRM;
}

bool WFixedBufferSession::Init(WNetWorkHandler* handler, uint32_t maxBufferSize, uint32_t messageSize)
{
    this->_maxBufferSize = maxBufferSize;
    this->_messageSize = messageSize;

    if ( !_recvBuffer.Init(maxBufferSize) )
    {
        return false;
    }
    if ( !_sendBuffer.Init(maxBufferSize) )
    {
        _recvBuffer.Destroy();
        return false;
    }

    this->_handler = handler;
    if (this->_handler == nullptr)
    {
        _recvBuffer.Destroy();
        _sendBuffer.Destroy();
        return false;
    }
    
    this->_op |= WNetWorkHandler::OP_IN;
    this->_op |= WNetWorkHandler::OP_ERR;
    this->_op |= WNetWorkHandler::OP_SHUT;
    this->_op |= WNetWorkHandler::OP_CLOS;

    return true;
}

bool WFixedBufferSession::SetConnectedSocket(base_socket_type socket, const WEndPointInfo& peerInfo)
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
    if ( !_handler->AddSocket(this, this->_socket, this->_op))
    {
        return false;
    }

    this->_peerInfo = peerInfo;

    this->_isConnected = true;

    return true;
}

void WFixedBufferSession::Close()
{
    // close
    this->_isConnected = false;
    ::close(this->_socket);
    this->_socket = -1;

    // clear
    this->Clear();
}

void WFixedBufferSession::Clear()
{
    this->_isConnected = false;
    _recvBuffer.Clear();
    _sendBuffer.Clear();
}

void WFixedBufferSession::Destroy()
{
    _recvBuffer.Destroy();
    _sendBuffer.Destroy();
    this->_listener = nullptr;
}

bool WFixedBufferSession::Send(const std::string& message)
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
    if ( !_handler->ModifySocket(this->_socket, this->_op) )
    {
        return false;
    }
    
    return true;
}

bool WFixedBufferSession::Receive()
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

void WFixedBufferSession::HandleError(int error_code)
{
    this->_errorMessage.clear();
    this->_errorMessage.assign(::strerror(error_code));

}

void WFixedBufferSession::OnError(int error_no)
{
    HandleError(error_no);
    
    if (this->_listener != nullptr)
    {
        bool ok = this->_listener->OnSessionError(this->_socket, error_no);
        
    }
    
    this->Close();
    return;
}

void WFixedBufferSession::OnClosed()
{
    if (this->_listener != nullptr)
    {
        this->_listener->OnSessionClosed(this->_socket);
    }
    this->Close();
    return;
}

void WFixedBufferSession::OnRead()
{
    if (!this->Receive())
    {
        ::shutdown(this->_socket, SHUT_RD);
        return;
    }
    
    std::string receive_message;
    std::string send_message;
    // on message
    while (1)
    {
        receive_message.clear();

        uint32_t len = this->_recvBuffer.GetFrontMessage(receive_message, this->_messageSize);
        
        if (len != this->_messageSize)
        {
            
            return;
        }
        

        if (!this->_listener->OnSessionMessage(this->_socket, receive_message, send_message))
        {
            ::shutdown(this->_socket, SHUT_RDWR);
            return;
        }
        if ( !send_message.empty() )
        {
            this->Send(send_message);
        }
    }
}

void WFixedBufferSession::OnWrite()
{
    std::string send_message;
    uint32_t msg_len = _sendBuffer.GetAllMessage(send_message);
    ssize_t send_len = ::send(this->_socket, send_message.c_str(), msg_len, 0);
    
    this->_sendBuffer.UpdateReadOffset(send_len);

    if (send_len < 0)
    {
        ::shutdown(this->_socket, SHUT_RDWR);
        return;
    }

    if (_sendBuffer.Empty())
    {
        this->_op -= WNetWorkHandler::OP_OUT;
        this->_handler->ModifySocket(this->_socket, this->_op);
    }

    return;
}

void WFixedBufferSession::OnShutdown()
{
    if (this->_listener != nullptr)
    {
        this->_listener->OnSessionShutdown(this->_socket);
    }
    
    if ( ::shutdown(this->_socket, SHUT_RDWR) == -1 )
    {
        
        return;
    }
    return;
}



}

