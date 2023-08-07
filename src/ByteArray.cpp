#include "wutils/ByteArray.h"
#include <cassert>
#include <iostream>

namespace wutils {

ByteArray::ByteArray() : buffer_() {}
ByteArray::ByteArray(uint64_t len, uint8_t value) : buffer_(len, value) {}
ByteArray::ByteArray(const uint8_t *data, uint64_t len) : buffer_(data, data + len) {}
ByteArray::ByteArray(ByteArray &&other) noexcept : buffer_(std::move(other.buffer_)) {}
ByteArray::ByteArray(const ByteArray &other) : buffer_(other.buffer_) {}


void ByteArray::append(const uint8_t *data, uint64_t len) {
    auto olen = this->buffer_.size();

    this->buffer_.resize(olen + len);

    std::copy(data, data + len, &this->buffer_[olen]);
}

void ByteArray::append(ByteArrayView view) {
    auto olen = this->buffer_.size();

    this->buffer_.resize(olen + view.size());

    std::copy(view.data(), view.data() + view.size(), &this->buffer_[olen]);
}

void ByteArray::append(const ByteArray &other) {
    auto old_len = this->buffer_.size();
    this->buffer_.resize(old_len + other.size());
    std::copy(other.buffer_.begin(), other.buffer_.end(), this->buffer_.data() + old_len);
}
ByteArray ByteArray::sliced(uint64_t start, uint64_t len) {
    assert(start + len <= this->buffer_.size());
    return ByteArray(this->data() + start, len);
}
ByteArray ByteArray::sliced(uint64_t start) { return ByteArray(this->data() + start, this->size() - start); }

ByteArrayView::ByteArrayView(const ByteArray &array) : pointer_(array.data()), len_(array.size()) {
    assert(pointer_);
    assert(len_);
}

ByteArrayView::ByteArrayView(const uint8_t *data, uint64_t len) : pointer_(data), len_(len) {}

ByteArray ByteArrayView::toByteArray() { return ByteArray(this->pointer_, this->len_); }

ByteArrayView ByteArrayView::sliced(uint64_t start, uint64_t len) {
    assert(start + len <= len_);
    return {this->pointer_ + start, len};
}

ByteArrayView ByteArrayView::sliced(uint64_t start) { return {this->pointer_ + start, this->len_ - start}; }

} // namespace wutils
