#include "WBuffer.hpp"
#include <iostream>

namespace wlb
{

bool RingBuffer::Init(uint32_t maxBufferSize)
{
    this->_maxBufferSize = maxBufferSize;
    this->_buffer = new(std::nothrow) char[maxBufferSize];

    if ( this->_buffer == nullptr)
    {
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
        delete[] this->_buffer;
        this->_buffer = nullptr;
    }
    
}

const uint32_t RingBuffer::GetRestBufferSize()
{
    return 0;
}

char* RingBuffer::GetBuffer()
{
    return this->_buffer;
}

void RingBuffer::UpdateWriteOffset(uint32_t len)
{
    std::string s;
    s.assign(this->_buffer, len);
    this->_WriteOffset += len;
    this->_WriteOffset /= this->_maxBufferSize;
}

void RingBuffer::UpdateReadOffset(uint32_t len)
{
    this->_ReadOffset += len;
    this->_ReadOffset /= this->_maxBufferSize;
}

uint32_t RingBuffer::InsertMessage(const std::string& message)
{
    return 0;
}

const std::string RingBuffer::GetFrontMessage(char* buf, uint32_t len)
{
    return "";
}

const uint32_t RingBuffer::GetAllMessage(std::string& message)
{
    return 0;
}

}

