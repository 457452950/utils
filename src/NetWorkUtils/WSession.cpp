#include "../../include/WSession.hpp"
#include <iostream>

namespace wlb::NetWork
{

/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Accepter
////////////////////////////////////////////////////////

bool WNetAccepter::Init(WNetWorkHandler* handler, const std::string& IpAddress, uint16_t port)
{
    /////////////////////////////////
    // Initialize socket
    this->_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);        // tcp v4

    if ( !SetSocketReuseAddr(this->_socket) || 
            !SetSocketReusePort(this->_socket) || 
            !SetSocketKeepAlive(this->_socket) )
    {
        std::cout << "WNetAccepter::Init SetSocketopt failed " << std::endl;
        return false;
    }
    
    if ( !SetSocketNoBlock(this->_socket) )
    {
        std::cout << "WNetAccepter::Init SetSocketNoBlock() failed" << std::endl;
    }
    std::cout << "WNetAccepter::Init SetSocketNoBlock() succ" << std::endl;

    // ///////////////////////////////////////
    // Init members
    this->_address = IpAddress; // copy
    this->_port = port;
    
    this->_handler = handler;
    if (this->_handler == nullptr)
    {
        std::cout << "WNetAccepter::Init handler nullptr" << std::endl;
        return false;
    }
    std::cout << "WNetAccepter::Init handler succ" << std::endl;

    // ///////////////////////////////////////
    // Bind 
    if ( !this->Bind() )
    {
        std::cout << "bind failed" << std::endl;
        return false;
    }
    std::cout << "bind succ" << std::endl;

    /////////////////////////////////
    // Listen
    if ( !this->Listen() )
    {
        std::cout << "Listen failed" << std::endl;
        return false;
    }
    std::cout << "Listen succ" << std::endl;
    
    // //////////////////////////////
    // add socket in handler
    uint32_t op = 0;
    op |= WNetWorkHandler::OP_IN;
    op |= WNetWorkHandler::OP_ERR;
    std::cout << "listen socket op:" << op << std::endl;
    if ( !this->_handler->AddSocket(this->_socket, op) )
    {
        std::cout << "WNetAccepter::Init AddSocket failed" << std::endl;
        return false;
    }
    std::cout << "WNetAccepter::Init AddSocket succ" << std::endl;
    
    return true;
}

bool WNetAccepter::Bind()
{
    sockaddr_in ei{0};
    ei.sin_family   = AF_INET;

    in_addr ipaddr{0};
    if ( !StringToIpAddress(this->_address, ipaddr))
    {
        std::cout << "WNetAccepter::Bind StringToIpAddr() failed" << std::endl;
        return false;
    }
    std::cout << "WNetAccepter::Bind StringToIpAddr() succ " << ipaddr.s_addr << std::endl;

    try
    {
        ei.sin_port = ::htons(this->_port);
        std::cout << "WNetAccepter::Bind port :" << ei.sin_port << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    std::cout << "WNetAccepter::Bind htons succ" << std::endl;

    int32_t ok = ::bind(this->_socket,
                (struct sockaddr*)&(ei),
                sizeof(ei));
    if ( ok == -1 )
    {
        std::cout << "WNetAccepter::Bind bind failed" << strerror(errno) << std::endl;
        close(this->_socket);
        return false;
    }
    std::cout << "WNetAccepter::Bind bind succ" << std::endl;

    return true;
}

bool WNetAccepter::Listen()
{
    if ( ::listen(this->_socket, 1024) == -1) 
    {
        std::cout << "WNetAccepter::Listen error " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

base_socket_type WNetAccepter::Accept()
{
    sockaddr_in client_info;
    socklen_t len = sizeof(client_info);
    base_socket_type clientsock = ::accept(
                                    this->_socket,
                                    (struct sockaddr*)&client_info,
                                    &len);
    if (clientsock <= 0)
    {
        return 0;
    }
    return clientsock;
}

base_socket_type WNetAccepter::GetListenSocket()
{
    return this->_socket;
}











/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Seession
////////////////////////////////////////////////////////

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
        return false;
    }

    this->_handler = handler;
    if (this->_handler == nullptr)
    {
        return false;
    }
    
    return true;
}

bool WFloatBufferSession::SetSocket(base_socket_type socket)
{
    this->_socket = socket;
    if ( !SetSocketNoBlock(this->_socket))
    {
        return false;
    }
    if ( !_handler->AddSocket(this->_socket, this->_op))
    {
        return false;
    }
    return true;
}

void WFloatBufferSession::Close()
{
    ::close(this->_socket);
    this->Clear();
}

void WFloatBufferSession::Clear()
{
    _recvBuffer.Clear();
    _sendBuffer.Clear();
}

void WFloatBufferSession::Destroy()
{
    _recvBuffer.Destroy();
    _sendBuffer.Destroy();
}

bool WFloatBufferSession::Send(const std::string& message)
{
    uint32_t msgSize = message.size();

    std::string head;
    if (this->_headLen == 2)
    {
        head = MakeWlbHead(wlbHead2(msgSize));
    }
    else if (this->_headLen == 4)
    {
        head = MakeWlbHead(wlbHead4(msgSize));
    }
    
    uint32_t insert_len = _sendBuffer.InsertMessage(head);
    if (insert_len != this->_headLen)
    {
        return false;
    }

    // 添加进send
    this->_op |= WNetWorkHandler::OP_OUT;
    if ( !_handler->ModifySocket(this->_socket, this->_op) )
    {
        return false;
    }
    
    return true;
}

bool WFloatBufferSession::Receive()
{
    uint32_t recv_len = ::recv(this->_socket, 
                                this->_recvBuffer.GetBuffer(), 
                                this->_recvBuffer.GetRestBufferSize(),
                                0);
    
    if (recv_len < -1)
    {
        this->Close();
    }
    
    this->_recvBuffer.UpdateWriteOffset(recv_len);
}

void WFloatBufferSession::HandleError()
{
    
}

bool WFloatBufferSession::OnError(base_socket_type socket, std::string error)
{
    HandleError();
    return false;
}

bool WFloatBufferSession::OnClosed(base_socket_type socket)
{
    return false;
}

bool WFloatBufferSession::OnRead(base_socket_type socket)
{
    return this->Receive();
}

bool WFloatBufferSession::OnWrite(base_socket_type socket)
{
    std::string send_message;
    uint32_t msg_len = _sendBuffer.GetAllMessage(send_message);
    uint32_t send_len = ::send(this->_socket, send_message.c_str(), msg_len, 0);

    if (send_len < 0)
    {
        this->Close();
        return false;
    }
    
    _sendBuffer.UpdateReadOffset(send_len);
    return true;
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

}

