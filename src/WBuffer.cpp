#include "WBuffer.hpp"
#include <iostream>
#include "WDebugger.hpp"

namespace wlb {

using namespace debug;

RingBuffer::RingBuffer() = default;

RingBuffer::~RingBuffer() {
    this->Destroy();
}

bool RingBuffer::Init(uint32_t maxBufferSize) {
    this->maxBufferSize_ = maxBufferSize;
    this->buffer_        = new(std::nothrow) char[maxBufferSize];
    NEWADD;
    NEWARRAYADD;

    if (this->buffer_ == nullptr) {
        this->errorMessage_ = "bad alloc, new char[] failed";
        return false;
    }
    return true;
}

void RingBuffer::Clear() {
    this->readOffset_  = 0;
    this->writeOffset_ = 0;
}

void RingBuffer::Destroy() {
    if (this->buffer_ != nullptr) {
        DELADD;
        DELARRAYADD;
        delete[] this->buffer_;
        this->buffer_ = nullptr;
    }

}

uint32_t RingBuffer::GetRestBufferSize() const {
    if (this->isFull_) {
        return 0;
    }

    return (this->writeOffset_ >= this->readOffset_) ?
           (this->maxBufferSize_ - this->writeOffset_ + this->readOffset_) :
           (this->readOffset_ - this->writeOffset_);
}

uint32_t RingBuffer::GetTopRestBufferSize() const {
    if (this->isFull_) {
        return 0;
    }

    return (this->writeOffset_ >= this->readOffset_) ?
           (this->maxBufferSize_ - this->writeOffset_) :
           (this->readOffset_ - this->writeOffset_);
}

char *RingBuffer::GetBuffer() {
    return this->buffer_;
}

char *RingBuffer::GetRestBuffer() {
    return this->buffer_ + this->writeOffset_;
}

void RingBuffer::UpdateWriteOffset(uint32_t len) {
    if (len == 0) {
        return;
    }

    this->writeOffset_ += len;
    this->writeOffset_ %= this->maxBufferSize_;

    this->isEmpty_ = false;
    if (this->writeOffset_ == this->readOffset_) {
        this->isFull_ = true;
    }
    // std::cout << "RingBuffer::UpdateWriteOffset " << this->writeOffset_ << "/" << this->readOffset_ << std::endl;
}

void RingBuffer::UpdateReadOffset(uint32_t len) {
    if (len == 0) {
        return;
    }

    this->readOffset_ += len;
    this->readOffset_ %= this->maxBufferSize_;

    this->isFull_ = false;
    if (this->writeOffset_ == this->readOffset_) {
        this->isEmpty_ = true;
    }
    // std::cout << "Write offset: " << this->writeOffset_ << " Read offset: " << this->readOffset_ << std::endl;
}

uint32_t RingBuffer::InsertMessage(const std::string &message) {
    uint32_t   top_size = this->GetTopRestBufferSize();
    const char *tmp     = message.c_str();
    uint32_t   tmp_size = message.size();

    if (this->GetRestBufferSize() < tmp_size) {
        return 0;
    }

    for (; tmp_size > 0;) {
        try {
            uint32_t cp_size = tmp_size > top_size ? top_size : tmp_size;

            ::memcpy(this->buffer_ + this->writeOffset_, tmp, cp_size);
            this->UpdateWriteOffset(cp_size);

            tmp_size -= cp_size;
            tmp += cp_size;
            top_size         = this->GetTopRestBufferSize();
        }
        catch (const std::exception &e) {
            this->errorMessage_ = e.what();
            return 0;
        }
    }
    return message.size();
}
uint32_t RingBuffer::GetFrontMessageLength() const {
    if (this->writeOffset_ > this->readOffset_) {
        return this->writeOffset_ - this->readOffset_;
    } else {
        return this->writeOffset_ + this->maxBufferSize_ - this->readOffset_;
    }
    return 0;
}
uint32_t RingBuffer::GetFrontMessage(std::string *message, uint32_t len) {
    message->clear();
    if (this->isEmpty_) {
        return 0;
    }

    if (this->writeOffset_ > this->readOffset_) {
        uint32_t msg_size = this->writeOffset_ - this->readOffset_;
        if (len > msg_size) {
            std::cout << "no enough msgsize" << msg_size << " l" << len << std::endl;
            return 0;   // Not enough message
        }
        message->append(this->buffer_ + this->readOffset_, len);
    } else // this->writeOffset_ <= this->readOffset_
    {
        uint32_t front_size = this->writeOffset_;
        uint32_t back_size  = this->maxBufferSize_ - this->readOffset_;

        if (front_size + back_size < len) {
            std::cout << "no enough fsize" << front_size << "bsize" << back_size << " l" << len << std::endl;
            return 0; // Not enough message
        }

        uint32_t cp_size          = back_size > len ? len : back_size;
        uint32_t temp_read_offset = this->readOffset_;
        while (cp_size != 0) {
            std::cout << "cp_size" << cp_size << std::endl;
            message->append(this->buffer_ + temp_read_offset, cp_size);
            temp_read_offset = (temp_read_offset + cp_size) % this->maxBufferSize_; // 更新临时读指针
            len -= cp_size;
            cp_size          = len;
        }
    }

    return message->size();
}

uint32_t RingBuffer::GetAllMessage(std::string *message) {
    if (this->isEmpty_) {
        return 0;
    }

    message->clear();

    uint32_t cp_size = 0;
    if (this->writeOffset_ > this->readOffset_) {
        cp_size = this->writeOffset_ - this->readOffset_;
        message->append(this->buffer_ + this->readOffset_, cp_size);
    } else {
        uint32_t front_size = this->writeOffset_;
        uint32_t back_size  = this->maxBufferSize_ - this->readOffset_;
        message->append(this->buffer_ + this->readOffset_, back_size);
        message->append(this->buffer_, front_size);
        cp_size = front_size + back_size;
    }

    return cp_size;
}

}

