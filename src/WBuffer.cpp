#include "WBuffer.hpp"
#include <iostream>
#include "WDebugger.hpp"

namespace wlb
{

using namespace debug;

#define RINGBUFADD \
DEBUGADD("RingBuffer")
#define RINGBUFRM \
DEBUGRM("RingBuffer")


RingBuffer::RingBuffer()
{
    RINGBUFADD;
}

RingBuffer::~RingBuffer()
{
    this->Destroy();
    RINGBUFRM;
}

bool RingBuffer::Init(uint32_t maxBufferSize)
{
    this->_maxBufferSize = maxBufferSize;
    this->_buffer = new(std::nothrow) char[maxBufferSize];
    NEWADD;
    NEWARRAYADD;

    if ( this->_buffer == nullptr)
    {
        this->_errorMessage = "new char[] failed";
        return false;
    }
    return true;
}

void RingBuffer::Clear()
{
    this->_ReadOffset = 0;
    this->_WriteOffset = 0;
}

void RingBuffer::Destroy()
{
    if (this->_buffer != nullptr)
    {
        DELADD;
        DELARRAYADD;
        delete[] this->_buffer;
        this->_buffer = nullptr;
    }
    
}

const uint32_t RingBuffer::GetRestBufferSize()
{
    if (this->_isFull)
    {
        return 0;
    }
    
    return (this->_WriteOffset >= this->_ReadOffset) ?
            (this->_maxBufferSize - this->_WriteOffset + this->_ReadOffset) :
            (this->_ReadOffset - this->_WriteOffset);
}

const uint32_t RingBuffer::GetTopRestBufferSize()
{
    if (this->_isFull)
    {
        return 0;
    }
    
    return (this->_WriteOffset >= this->_ReadOffset) ? 
                        (this->_maxBufferSize - this->_WriteOffset) : 
                        (this->_ReadOffset - this->_WriteOffset);
}

char* RingBuffer::GetBuffer()
{
    return this->_buffer;
}

char* RingBuffer::GetRestBuffer()
{
    return this->_buffer + this->_WriteOffset;
}


void RingBuffer::UpdateWriteOffset(uint32_t len)
{
    if (len == 0)
    {
        return;
    }
    
    this->_WriteOffset += len;
    this->_WriteOffset %= this->_maxBufferSize;

    this->_isEmpty = false;
    if (this->_WriteOffset == this->_ReadOffset)
    {
        this->_isFull == true;
    }
    // std::cout << "RingBuffer::UpdateWriteOffset " << this->_WriteOffset << "/" << this->_ReadOffset << std::endl;
}

void RingBuffer::UpdateReadOffset(uint32_t len)
{
    if (len == 0)
    {
        return;
    }
    
    this->_ReadOffset += len;
    this->_ReadOffset %= this->_maxBufferSize;

    this->_isFull = false;
    if (this->_WriteOffset == this->_ReadOffset)
    {
        this->_isEmpty = true;
    }
    // std::cout << "Write offset: " << this->_WriteOffset << " Read offset: " << this->_ReadOffset << std::endl;
}

uint32_t RingBuffer::InsertMessage(const std::string& message)
{
    uint32_t top_size = this->GetTopRestBufferSize();
    const char* tmp = message.c_str();
    uint32_t tmp_size = message.size();

    if (this->GetRestBufferSize() < tmp_size)
    {
        return 0;
    }
    
    for ( ; tmp_size > 0; )
    {
        try
        {
            uint32_t cp_size = tmp_size > top_size ? top_size : tmp_size;

            ::memcpy(this->_buffer + this->_WriteOffset, tmp, cp_size);
            this->UpdateWriteOffset(cp_size);

            tmp_size -= cp_size;
            tmp += cp_size;
            top_size = this->GetTopRestBufferSize();
        }
        catch(const std::exception& e)
        {
            this->_errorMessage = e.what();
            return 0;
        }
    }
    return message.size();
}
const uint32_t RingBuffer::GetFrontMessageLength()
{
    if (this->_WriteOffset > this->_ReadOffset)
    {
        return this->_WriteOffset - this->_ReadOffset;
    }
    else
    {
        return this->_WriteOffset + this->_maxBufferSize - this->_ReadOffset;
    }
    return 0;
}
const uint32_t RingBuffer::GetFrontMessage(std::string& message, uint32_t len)
{
    message.clear();
    if (this->_isEmpty)
    {
        return 0;
    }
    
    if (this->_WriteOffset > this->_ReadOffset)
    {
        uint32_t msg_size = this->_WriteOffset - this->_ReadOffset;
        if (len > msg_size)
        {
            std::cout << "no enough msgsize" << msg_size << " l" << len << std::endl;
            return 0;   // Not enough message
        }
        message.append(this->_buffer + this->_ReadOffset, len);
    }
    else // this->_WriteOffset <= this->_ReadOffset
    {
        uint32_t front_size = this->_WriteOffset;
        uint32_t back_size = this->_maxBufferSize - this->_ReadOffset;

        if (front_size + back_size < len)
        {
            std::cout << "no enough fsize" << front_size << "bsize" << back_size << " l" << len << std::endl;
            return 0; // Not enough message
        }
        
        uint32_t cp_size = back_size > len ? len : back_size;
        uint32_t temp_read_offset = this->_ReadOffset;
        while (cp_size != 0)
        {
            std::cout << "cp_size" << cp_size << std::endl;
            message.append(this->_buffer + temp_read_offset, cp_size);
            temp_read_offset = (temp_read_offset + cp_size) % this->_maxBufferSize; // 更新临时读指针
            len -= cp_size;
            cp_size = len;
        }
    }
    
    return message.size();
}

const uint32_t RingBuffer::GetAllMessage(std::string& message)
{
    if (this->_isEmpty)
    {
        return 0;
    }

    message.clear();

    uint32_t cp_size = 0;
    if (this->_WriteOffset > this->_ReadOffset)
    {
        cp_size = this->_WriteOffset - this->_ReadOffset;
        message.append(this->_buffer + this->_ReadOffset, cp_size);
    }
    else
    {
        uint32_t front_size = this->_WriteOffset;
        uint32_t back_size = this->_maxBufferSize - this->_ReadOffset;
        message.append(this->_buffer + this->_ReadOffset, back_size);
        message.append(this->_buffer, front_size);
        cp_size = front_size + back_size;
    }
    
    
    return cp_size;
}

}

