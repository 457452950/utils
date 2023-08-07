#include "wutils/buffer/ChainBuffer.h"

#include <cassert>

namespace wutils {

ChainBuffer::ChainBuffer() = default;

ChainBuffer::ChainBuffer(ChainBuffer &&other) noexcept : pages_(std::move(other.pages_)) {}

bool ChainBuffer::Init(uint64_t) {
    this->pages_.clear();
    this->pages_.emplace_back();
    data_size_ = 0;
    return true;
}

void ChainBuffer::Release() {
    this->pages_.clear();
    data_size_ = 0;
}

bool ChainBuffer::IsFull() const { return this->UsedBytes() >= this->MaxSize(); }

bool ChainBuffer::IsEmpty() const {
    assert(!this->pages_.empty());

    auto it = this->pages_.begin();
    return it->w_offset == it->r_offset;
}

uint64_t ChainBuffer::GetReadableBytes() const {
    auto it = this->pages_.begin();
    return it->w_offset - it->r_offset;
}

uint64_t ChainBuffer::GetWriteableBytes() const {
    assert(!checkLastPageIsFull());

    auto it = this->pages_.rbegin();
    return it->page_size - it->w_offset;
}

uint8_t *ChainBuffer::PeekRead() {
    assert(!this->pages_.empty());

    auto it = this->pages_.begin();
    return it->start.get() + it->r_offset;
}

uint8_t *ChainBuffer::PeekWrite() {
    assert(!checkLastPageIsFull());

    auto it = this->pages_.rbegin();
    return it->start.get() + it->w_offset;
}

const uint8_t *ChainBuffer::ConstPeekRead() const {
    assert(!this->pages_.empty());

    auto it = this->pages_.begin();
    return it->start.get() + it->r_offset;
}

const uint8_t *ChainBuffer::ConstPeekWrite() const {
    assert(!checkLastPageIsFull());

    auto it = this->pages_.rbegin();
    return it->start.get() + it->w_offset;
}

void ChainBuffer::SkipReadBytes(uint64_t bytes) {
    assert(bytes <= data_size_);

    do {
        auto     it  = this->pages_.begin();
        uint64_t len = std::min(it->w_offset - it->r_offset, bytes);

        // skipped bytes bigger than this->data_size
        assert(len);

        it->r_offset += len;

        assert(it->r_offset <= it->w_offset);

        if(it->r_offset == it->page_size) {
            this->pages_.erase(it);
        }

        bytes      -= len;
        data_size_ -= len;
    } while(bytes);
}

void ChainBuffer::SkipAllReadBytes() {
    this->pages_.clear();
    this->pages_.emplace_back();
    data_size_ = 0;
}

void ChainBuffer::UpdateWriteBytes(uint64_t bytes) {
    auto it       = this->pages_.rbegin();
    it->w_offset += bytes;

    assert(it->w_offset <= it->page_size);

    if(it->page_size == it->w_offset) {
        this->pages_.emplace_back();
    }

    data_size_ += bytes;
}

bool ChainBuffer::Write(const uint8_t *data, uint64_t bytes) {
    if(this->IsFull()) {
        return false;
    }
    this->WriteSome(data, bytes);
    return true;
}

bool ChainBuffer::Write(unique_ptr<uint8_t[]> data, uint64_t bytes) {
    if(this->IsFull()) {
        return false;
    }

    assert(!this->checkLastPageIsFull());
    auto page = std::prev(this->pages_.end());

    if(checkLastPageIsEmpty()) {
        this->pages_.insert(page, Page(std::move(data), bytes));
    } else {
        page->page_size = page->w_offset;
        this->pages_.emplace_back(std::move(data), bytes);
        this->pages_.emplace_back();
    }

    data_size_ += bytes;
    return true;
}

bool ChainBuffer::Read(uint8_t *buffer, uint64_t buffer_len) {
    assert(!this->pages_.empty());

    if(data_size_ < buffer_len) {
        return false;
    }

    this->ReadSome(buffer, buffer_len);
    return true;
}

uint64_t ChainBuffer::WriteSome(const uint8_t *data, uint64_t bytes) {
    uint64_t l = 0;

    if(this->checkLastPageIsFull()) {
        this->pages_.emplace_back();
    }

    do {
        auto it  = this->pages_.rbegin();
        auto len = std::min(bytes, it->page_size - it->w_offset);

        std::copy(data + l, data + l + len, it->start.get() + it->w_offset);

        this->UpdateWriteBytes(len);

        l     += len;
        bytes -= len;
    } while(bytes);

    return l;
}

uint64_t ChainBuffer::ReadSome(uint8_t *buffer, uint64_t buffer_len) {
    uint64_t l = 0;

    do {
        auto it  = this->pages_.begin();
        auto len = std::min(buffer_len, it->w_offset - it->r_offset);

        if(len == 0) {
            // no data read
            break;
        }

        std::copy(it->start.get() + it->r_offset, it->start.get() + it->r_offset + len, buffer + l);

        this->SkipReadBytes(len);

        l          += len;
        buffer_len -= len;
    } while(true);

    return l;
}


} // namespace wutils
