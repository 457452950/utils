#include "../../include/WSession.hpp"

namespace wlb::NetWork
{

bool WFloatBufferSession::Init(uint32_t maxBufferSize, uint32_t headLen)
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

    return true;
}

void WFloatBufferSession::Close()
{
    WBaseSession::Close();
    _recvBuffer.Clear();
    _sendBuffer.Clear();
}

void WFloatBufferSession::Destroy()
{
    WBaseSession::Destroy();
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
    

    uint32_t insert_len = _sendBuffer.InsertMessage(message);
    if (insert_len != msgSize)
    {
        // error sending
        return false;
    }

    // 添加进send
    _handler->AddSocket(this->_socket, this->_op);
    
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

