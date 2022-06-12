#include "WBuffer.hpp"
#include <iostream>
#include <cassert>

namespace wlb {

RingBuffer::RingBuffer() = default;

RingBuffer::~RingBuffer() {
    this->Destroy();
}

bool RingBuffer::Init(uint32_t maxBufferSize) {
    assert(maxBufferSize);

    this->max_buffer_size_ = maxBufferSize;
    this->buffer_          = new(std::nothrow) char[maxBufferSize];

    assert(this->buffer_);

    return true;
}

void RingBuffer::Clear() {
    this->read_offset_  = 0;
    this->write_offset_ = 0;
}

void RingBuffer::Destroy() {
    if (this->buffer_ != nullptr) {
        delete[] this->buffer_;
        this->buffer_ = nullptr;
    }

}

uint32_t RingBuffer::GetRestBufferSize() const {
    if (this->is_full_) {
        return 0;
    }

    return (this->write_offset_ >= this->read_offset_) ?
           (this->max_buffer_size_ - this->write_offset_ + this->read_offset_) :
           (this->read_offset_ - this->write_offset_);
}

uint32_t RingBuffer::GetTopRestBufferSize() const {
    if (this->is_full_) {
        return 0;
    }

    return (this->write_offset_ >= this->read_offset_) ?
           (this->max_buffer_size_ - this->write_offset_) :
           (this->read_offset_ - this->write_offset_);
}

char *RingBuffer::GetRestBuffer() {
    return this->buffer_ + this->write_offset_;
}

void RingBuffer::UpdateWriteOffset(uint32_t len) {
    if (len == 0) {
        return;
    }

    this->write_offset_ += len;

    assert(this->write_offset_ <= this->max_buffer_size_);

    this->write_offset_ %= this->max_buffer_size_;

    this->is_empty_ = false;
    if (this->write_offset_ == this->read_offset_) {
        this->is_full_ = true;
    }
    // std::cout << "RingBuffer::UpdateWriteOffset " << this->write_offset_ << "/" << this->read_offset_ << std::endl;
}

void RingBuffer::UpdateReadOffset(uint32_t len) {
    if (len == 0) {
        return;
    }

    this->read_offset_ += len;
    this->read_offset_ %= this->max_buffer_size_;

    this->is_full_ = false;
    if (this->write_offset_ == this->read_offset_) {
        this->is_empty_ = true;
    }
    // std::cout << "Write offset: " << this->write_offset_ << " Read offset: " << this->read_offset_ << std::endl;
}

uint32_t RingBuffer::InsertMessage(const std::string &message) {
    uint32_t   top_size = this->GetTopRestBufferSize();
    const char *tmp     = message.c_str();
    uint32_t   tmp_size = message.size();

    if (this->GetRestBufferSize() < tmp_size) {
        return 0;
    }

    for (; tmp_size > 0;) {
        uint32_t cp_size = tmp_size > top_size ? top_size : tmp_size;

        ::memcpy(this->buffer_ + this->write_offset_, tmp, cp_size);
        this->UpdateWriteOffset(cp_size);

        tmp_size -= cp_size;
        tmp += cp_size;
        top_size         = this->GetTopRestBufferSize();
    }
    return message.size();
}
uint32_t RingBuffer::GetFrontMessageLength() const {
    if (this->write_offset_ > this->read_offset_) {
        return this->write_offset_ - this->read_offset_;
    } else {
        return this->write_offset_ + this->max_buffer_size_ - this->read_offset_;
    }
    return 0;
}
uint32_t RingBuffer::GetFrontMessage(std::string *message, uint32_t len) {
    message->clear();
    if (this->is_empty_) {
        return 0;
    }

    if (this->write_offset_ > this->read_offset_) {
        uint32_t msg_size = this->write_offset_ - this->read_offset_;
        if (len > msg_size) {
//            std::cout << "no enough msg size" << msg_size << " l" << len << std::endl;
            return 0;   // Not enough message
        }
        message->append(this->buffer_ + this->read_offset_, len);
    } else // this->write_offset_ <= this->read_offset_
    {
        uint32_t front_size = this->write_offset_;
        uint32_t back_size  = this->max_buffer_size_ - this->read_offset_;

        if (front_size + back_size < len) {
//            std::cout << "no enough fsize" << front_size << "bsize" << back_size << " l" << len << std::endl;
            return 0; // Not enough message
        }

        uint32_t cp_size          = back_size > len ? len : back_size;
        uint32_t temp_read_offset = this->read_offset_;
        while (cp_size != 0) {
//            std::cout << "cp_size" << cp_size << std::endl;
            message->append(this->buffer_ + temp_read_offset, cp_size);
            temp_read_offset = (temp_read_offset + cp_size) % this->max_buffer_size_; // 更新临时读指针
            len -= cp_size;
            cp_size          = len;
        }
    }

    return message->size();
}

uint32_t RingBuffer::GetAllMessage(std::string *message) {
    if (this->is_empty_) {
        return 0;
    }

    message->clear();

    uint32_t cp_size = 0;
    if (this->write_offset_ > this->read_offset_) {
        cp_size = this->write_offset_ - this->read_offset_;
        message->append(this->buffer_ + this->read_offset_, cp_size);
    } else {
        uint32_t front_size = this->write_offset_;
        uint32_t back_size  = this->max_buffer_size_ - this->read_offset_;
        message->append(this->buffer_ + this->read_offset_, back_size);
        message->append(this->buffer_, front_size);
        cp_size = front_size + back_size;
    }

    return cp_size;
}

}

