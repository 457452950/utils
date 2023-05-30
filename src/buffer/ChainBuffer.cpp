#include "wutils/buffer/ChainBuffer.h"

#include <cassert>

namespace wutils {

ChainBuffer::ChainBuffer() { this->pages_.emplace_back(); }

ChainBuffer::ChainBuffer(const ChainBuffer &other) : pages_(other.pages_) {}

ChainBuffer::ChainBuffer(const ChainBuffer &&other) noexcept : pages_(std::move(other.pages_)) {}

ChainBuffer &ChainBuffer::operator=(const ChainBuffer &other) {
    if(this == &other) {
        return *this;
    }

    this->pages_ = other.pages_;

    return *this;
}

bool ChainBuffer::Init(uint64_t) {
    this->pages_.clear();
    this->check();
    return true;
}

void ChainBuffer::Release() { this->pages_.clear(); }

bool ChainBuffer::IsFull() const { return this->UsedBytes() >= this->MaxSize(); }

bool ChainBuffer::IsEmpty() const {
    auto it = this->pages_.begin();

    return it->w_offset == it->r_offset;
}

uint64_t ChainBuffer::GetReadableBytes() const {
    auto it = this->pages_.begin();

    return it->w_offset - it->r_offset;
}

uint64_t ChainBuffer::GetWriteableBytes() const {
    auto it = std::prev(this->pages_.end());
    return PAGE_SIZE - it->w_offset;
}

uint64_t ChainBuffer::UsedBytes() const {
    // clang-format off
    // uint64_t total = 0;
    // std::for_each(pages_.begin(), pages_.end(),
    //              [&total](const Page &page) {
    //                  total += (page.w_offset - page.r_offset);
    // });
    // return total;
    // clang-format on

    // be faster
    assert(!this->pages_.empty());

    auto bg = this->pages_.begin();
    auto bk = std::prev(this->pages_.end());
    return (this->pages_.size() - 1) * PAGE_SIZE + bk->w_offset - bg->r_offset;
}

uint64_t ChainBuffer::MaxSize() const { return MAX_BUFFER_SIZE; }

uint8_t *ChainBuffer::PeekRead() {
    auto it = this->pages_.begin();
    return it->start.data() + it->r_offset;
}

uint8_t *ChainBuffer::PeekWrite() {
    auto it = std::prev(this->pages_.end());
    return it->start.data() + it->w_offset;
}

const uint8_t *ChainBuffer::ConstPeekRead() const {
    auto it = this->pages_.begin();
    return it->start.data() + it->r_offset;
}

const uint8_t *ChainBuffer::ConstPeekWrite() const {
    auto it = std::prev(this->pages_.end());
    return it->start.data() + it->w_offset;
}

void ChainBuffer::SkipReadBytes(uint64_t bytes) {
    do {
        auto     it  = this->pages_.begin();
        uint64_t len = std::min((uint64_t)it->w_offset - it->r_offset, bytes);

        assert(len <= UINT16_MAX);

        it->r_offset += (uint16_t)len;
        if(it->r_offset != PAGE_SIZE) {
            break;
        } else {
            this->pages_.erase(it);
        }

        bytes -= len;
    } while(bytes);

    this->check();
}

void ChainBuffer::SkipAllReadBytes() {
    this->pages_.clear();
    this->check();
}

void ChainBuffer::UpdateWriteBytes(uint64_t bytes) {
    auto it = std::prev(this->pages_.end());
    it->w_offset += bytes;

    assert(it->w_offset <= PAGE_SIZE);

    this->check();
}

bool ChainBuffer::WriteFixBytes(const uint8_t *data, uint64_t bytes) {
    if(this->IsFull()) {
        return false;
    }
    this->Write(data, bytes);
    return true;
}

bool ChainBuffer::ReadFixBytes(uint8_t *buffer, uint64_t buffer_len) {
    if(this->UsedBytes() < buffer_len) {
        return false;
    }

    this->Read(buffer, buffer_len);
    return true;
}

uint64_t ChainBuffer::Write(const uint8_t *data, uint64_t bytes) {
    uint64_t l = 0;

    if(this->IsFull()) {
        return l;
    }

    do {
        auto it  = std::prev(this->pages_.end());
        auto len = std::min(bytes, (uint64_t)PAGE_SIZE - it->w_offset);

        assert(len < UINT16_MAX);

        std::copy(data + l, data + l + len, it->start.data() + it->w_offset);

        this->UpdateWriteBytes(len);

        l += len;
        bytes -= len;
    } while(bytes);

    return l;
}

uint64_t ChainBuffer::Read(uint8_t *buffer, uint64_t buffer_len) {
    uint64_t l = 0;

    do {
        auto it  = this->pages_.begin();
        auto len = std::min(buffer_len, (uint64_t)it->w_offset - it->r_offset);

        assert(len <= UINT16_MAX);

        if(len == 0) {
            // no data read or buffer full
            break;
        }

        std::copy(it->start.data() + it->r_offset, it->start.data() + it->r_offset + len, buffer + l);

        this->SkipReadBytes(len);

        l += len;
        buffer_len -= len;
    } while(true);

    return l;
}

void ChainBuffer::WriteUntil(writecb cb) {
    uint64_t len = 0;
    do {
        if(this->IsFull()) {
            break;
        }

        len = cb(this->PeekWrite(), this->GetWriteableBytes());
        this->UpdateWriteBytes(len);
    } while(len);
}

void ChainBuffer::ReadUntil(readcb cb) {
    uint64_t len = 0;
    do {
        len = cb(this->ConstPeekRead(), this->GetReadableBytes());
        this->SkipReadBytes(len);
    } while(len || this->IsEmpty());
}

void ChainBuffer::check() {
    if(this->pages_.empty()) {
        this->pages_.emplace_back();
        return;
    }

    auto it = std::prev(this->pages_.end());
    if(it->w_offset == PAGE_SIZE) {
        this->pages_.emplace_back();
    }
}

} // namespace wutils
