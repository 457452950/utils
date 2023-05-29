#include "wutils/buffer/straightbuffer.h"

#include <cassert>

Straightbuffer::Straightbuffer() {}

Straightbuffer::Straightbuffer(const Straightbuffer &other) : buffer_(other.buffer_), offset_(other.offset_) {}

Straightbuffer::Straightbuffer(const Straightbuffer &&other) :
    buffer_(std::move(other.buffer_)), offset_(other.offset_) {}

Straightbuffer &Straightbuffer::operator=(const Straightbuffer &other) {
    if(this == &other) {
        return *this;
    }

    if(this->MaxSize() != other.MaxSize())
        this->buffer_.resize(other.MaxSize());

    std::copy(other.ConstPeekRead(), other.ConstPeekWrite(), this->buffer_.begin());
    this->offset_ = other.offset_;
    return *this;
}

bool Straightbuffer::Init(uint64_t max_buffer_byte) {
    this->buffer_.resize(max_buffer_byte);
    return true;
}

void Straightbuffer::Release() {
    this->buffer_.clear();
    this->offset_ = 0;
}

bool Straightbuffer::IsFull() const { return this->offset_ == this->MaxSize(); }

bool Straightbuffer::IsEmpty() const { return this->offset_ == 0; }

uint64_t Straightbuffer::GetReadableBytes() const { return this->offset_; }

uint64_t Straightbuffer::GetWriteableBytes() const { return this->MaxSize() - this->offset_; }

uint64_t Straightbuffer::UsedBytes() const { return this->offset_; }

uint64_t Straightbuffer::MaxSize() const { return this->buffer_.size(); }

uint8_t *Straightbuffer::PeekRead() { return this->buffer_.data(); }

uint8_t *Straightbuffer::PeekWrite() { return this->buffer_.data() + this->offset_; }

const uint8_t *Straightbuffer::ConstPeekRead() const { return this->buffer_.data(); }

const uint8_t *Straightbuffer::ConstPeekWrite() const { return this->buffer_.data() + this->offset_; }

void Straightbuffer::SkipReadBytes(uint64_t bytes) {
    if(bytes >= this->offset_) {
        this->SkipAllReadBytes();
        return;
    }

    uint64_t len = this->offset_ - bytes;

    //    for(uint64_t i = 0; i < len; ++i) {
    //        this->buffer_[i] = this->buffer_[i + bytes];
    //    }
    std::copy(this->buffer_.data() + bytes, this->buffer_.data() + this->offset_, this->buffer_.data());

    this->offset_ -= bytes;
}

void Straightbuffer::SkipAllReadBytes() { this->offset_ = 0; }

void Straightbuffer::UpdateWriteBytes(uint64_t bytes) {
    assert(bytes + this->offset_ <= this->MaxSize());
    this->offset_ += bytes;
}

bool Straightbuffer::WriteFixBytes(const uint8_t *data, uint64_t bytes) {
    if(this->GetWriteableBytes() < bytes) {
        return false;
    }

    std::copy(data, data + bytes, this->buffer_.data() + this->offset_);
    this->UpdateWriteBytes(bytes);
    return true;
}

bool Straightbuffer::ReadFixBytes(uint8_t *buffer, uint64_t buffer_len) {
    if(this->GetReadableBytes() < buffer_len) {
        return false;
    }

    std::copy(this->buffer_.data(), this->buffer_.data() + buffer_len, buffer);
    this->SkipReadBytes(buffer_len);
    return true;
}

uint64_t Straightbuffer::Write(const uint8_t *data, uint64_t bytes) {
    auto len = std::min(bytes, this->GetWriteableBytes());

    std::copy(data, data + len, this->buffer_.data() + this->offset_);
    this->UpdateWriteBytes(len);
    return len;
}

uint64_t Straightbuffer::Read(uint8_t *buffer, uint64_t buffer_len) {
    auto len = std::min(buffer_len, this->GetWriteableBytes());

    std::copy(this->buffer_.data(), this->buffer_.data() + len, buffer);

    this->SkipReadBytes(len);

    return len;
}

void Straightbuffer::WriteUntil(writecb cb) {
    auto len = cb(this->PeekWrite(), this->GetWriteableBytes());
    assert(this->offset_ + len <= this->MaxSize());
    this->UpdateWriteBytes(len);
}

void Straightbuffer::ReadUntil(readcb cb) {
    auto len = cb(this->ConstPeekRead(), this->GetReadableBytes());
    assert(len > this->GetReadableBytes());
    this->SkipReadBytes(len);
}
