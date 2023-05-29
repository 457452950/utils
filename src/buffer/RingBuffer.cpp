#include "wutils/buffer/RingBuffer.h"

#include <cassert>

RingBuffer::RingBuffer() {}

// clang-format off
RingBuffer::RingBuffer(const RingBuffer &other) :
    buffer_(other.buffer_),
    read_offset_(other.read_offset_),
    write_offset_(other.write_offset_),
    is_full_(other.is_full_) {}

RingBuffer::RingBuffer(const RingBuffer &&other) noexcept :
    buffer_(std::move(other.buffer_)),
    read_offset_(other.read_offset_),
    write_offset_(other.write_offset_),
    is_full_(other.is_full_) {}
// clang-format on

RingBuffer &RingBuffer::operator=(const RingBuffer &other) {
    if(this == &other) {
        return *this;
    }

    if(other.IsFull()) {
        this->buffer_       = other.buffer_;
        this->read_offset_  = other.read_offset_;
        this->write_offset_ = other.write_offset_;
        this->is_full_      = other.is_full_;
        return *this;
    }

    // 减少 copy 量
    this->read_offset_  = 0;
    this->write_offset_ = 0;
    this->is_full_      = other.is_full_;
    if(this->MaxSize() != other.MaxSize())
        this->buffer_.resize(other.MaxSize());
    if(read_offset_ > write_offset_) {
        uint64_t after = other.MaxSize() - other.read_offset_;
        std::copy(other.ConstPeekRead(), other.buffer_.data() + other.MaxSize(), this->PeekWrite());
        std::copy(other.buffer_.data(), other.ConstPeekWrite(), this->PeekWrite() + after);
    } else {
        std::copy(other.ConstPeekRead(), other.ConstPeekWrite(), this->PeekWrite());
    }
    this->UpdateWriteBytes(other.UsedBytes());

    return *this;
}

bool RingBuffer::Init(uint64_t max_buffer_byte) {
    this->buffer_.resize(max_buffer_byte);
    return true;
}

void RingBuffer::Release() {
    this->buffer_.clear();
    this->read_offset_  = 0;
    this->write_offset_ = 0;
}

bool RingBuffer::IsFull() const { return this->is_full_; }

bool RingBuffer::IsEmpty() const {
    if(this->read_offset_ == this->write_offset_) {
        return !this->IsFull();
    } else {
        return false;
    }
}

uint64_t RingBuffer::GetReadableBytes() const {
    if(write_offset_ >= read_offset_) {
        if(is_full_) {
            return this->MaxSize();
        }
        return write_offset_ - read_offset_;
    } else {
        return this->MaxSize() - read_offset_;
    }
}

uint64_t RingBuffer::GetWriteableBytes() const {
    if(write_offset_ > read_offset_) {
        return this->MaxSize() - write_offset_;
    } else {
        if(this->IsEmpty()) {
            return this->MaxSize();
        }
        return read_offset_ - write_offset_;
    }
}

uint64_t RingBuffer::UsedBytes() const {
    if(this->IsFull()) {
        return this->MaxSize();
    }

    uint64_t max = this->MaxSize();
    return (max + write_offset_ - read_offset_) % max;
}

uint64_t RingBuffer::MaxSize() const { return this->buffer_.size(); }

uint8_t *RingBuffer::PeekRead() { return this->buffer_.data() + read_offset_; }

uint8_t *RingBuffer::PeekWrite() { return this->buffer_.data() + write_offset_; }

const uint8_t *RingBuffer::ConstPeekRead() const { return this->buffer_.data() + read_offset_; }

const uint8_t *RingBuffer::ConstPeekWrite() const { return this->buffer_.data() + write_offset_; }

void RingBuffer::SkipReadBytes(uint64_t bytes) {
    if(this->UsedBytes() <= bytes) {
        this->SkipAllReadBytes();
        return;
    }

    this->read_offset_ += bytes;
    this->read_offset_ %= this->MaxSize();

    if(bytes != 0 && this->is_full_) {
        this->is_full_ = false;
    }
}

void RingBuffer::SkipAllReadBytes() {
    this->write_offset_ = 0;
    this->read_offset_  = 0;
    this->is_full_      = false;
}

void RingBuffer::UpdateWriteBytes(uint64_t bytes) {
    assert((this->UsedBytes() + bytes) <= this->MaxSize());
    this->write_offset_ += bytes;
    this->write_offset_ %= this->MaxSize();

    if(write_offset_ == read_offset_) {
        this->is_full_ = true;
    }
}

bool RingBuffer::WriteFixBytes(const uint8_t *data, uint64_t bytes) {
    if((this->UsedBytes() + bytes) > this->MaxSize()) {
        return false;
    }
    this->Write(data, bytes);
    return true;
}

bool RingBuffer::ReadFixBytes(uint8_t *buffer, uint64_t buffer_len) {
    if(this->UsedBytes() < buffer_len) {
        return false;
    }
    this->Read(buffer, buffer_len);
    return true;
}

uint64_t RingBuffer::Write(const uint8_t *data, uint64_t bytes) {
    const auto len = std::min(bytes, this->MaxSize() - this->UsedBytes());

    if(write_offset_ + len >= this->MaxSize()) {
        //
        auto len_ = MaxSize() - write_offset_;

        std::copy(data, data + len_, this->PeekWrite());
        std::copy(data + len_, data + len, this->buffer_.data());
    } else {
        //
        std::copy(data, data + len, this->PeekWrite());
    }

    this->UpdateWriteBytes(len);

    return len;
}

uint64_t RingBuffer::Read(uint8_t *buffer, uint64_t buffer_len) {
    const auto len = std::min(buffer_len, this->UsedBytes());

    if(read_offset_ + len > this->MaxSize()) {
        //
        auto len_ = MaxSize() - read_offset_;

        std::copy(this->PeekRead(), this->PeekRead() + len_, buffer);
        std::copy(this->buffer_.data(), this->buffer_.data() + len - len_, buffer + len_);

    } else {
        std::copy(this->PeekRead(), this->PeekRead() + len, buffer);
    }

    this->SkipReadBytes(len);
    return len;
}

void RingBuffer::WriteUntil(writecb cb) {
    uint64_t len = 0;

    do {
        len = cb(this->PeekWrite(), this->GetWriteableBytes());
        this->UpdateWriteBytes(len);
    } while(!this->IsFull() && len != 0);
}

void RingBuffer::ReadUntil(readcb cb) {
    uint64_t len = 0;

    do {
        len = cb(this->ConstPeekRead(), this->GetReadableBytes());
        this->SkipReadBytes(len);
    } while(!this->IsEmpty() && len != 0);
}
