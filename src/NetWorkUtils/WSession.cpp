#include "../../include/WSession.hpp"

namespace wlb::NetWork
{

/////////////////////////////////////////////////////////
// Created by wlb on 2021/9/17.
// Accepter
////////////////////////////////////////////////////////

bool WNetAccepter::Init(WNetWorkHandler* handler, const std::string& IpAddress, uint16_t port)
{
    this->_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);        // tcp v4

    int opt = 1;
    unsigned int len = sizeof(opt);
    try
    {
        ::setsockopt(this->_socket, SOL_SOCKET, SO_REUSEADDR, &opt, len);
        ::setsockopt(this->_socket, SOL_SOCKET, SO_REUSEPORT, &opt, len);
        ::setsockopt(this->_socket, SOL_SOCKET, SO_KEEPALIVE, &opt, len);
    }
    catch(const std::exception& e)
    {
        
    }
    if ( !SetSocketNoBlock(this->_socket) )
    {
        ;
    }
    
    sockaddr_in ei;
    ei.sin_family   = AF_INET;
    in_addr ipaddr;
    if ( !StringToIpAddress(IpAddress, ipaddr))
    {
        return false;
    }
    ei.sin_port = ::htons(port);

    auto ok = ::bind(this->_socket,
                (struct sockaddr * )&(ei),
                sizeof(ei));

    if ( !ok )
    {
        close(this->_socket);
        return false;
    }
    this->_handler = handler;
    this->_handler->AddSocket(this->_socket, WNetWorkHandler::OP_IN | WNetWorkHandler::OP_ERR);
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

