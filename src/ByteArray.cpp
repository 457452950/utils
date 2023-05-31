#include "wutils/ByteArray.h"

namespace wutils {

ByteArray::ByteArray() : buffer_() {}
ByteArray::ByteArray(uint64_t len, uint8_t value) : buffer_(len, value) {}
ByteArray::ByteArray(const uint8_t *data, uint64_t len) : buffer_(data, data + len) {}


void ByteArray::append(uint8_t *data, uint64_t len) {
    auto olen = this->buffer_.size();

    this->buffer_.resize(olen + len);

    std::copy(data, data + len, &this->buffer_[olen]);
}

void ByteArray::append(ByteArrayView view) {
    auto olen = this->buffer_.size();

    this->buffer_.resize(olen + view.size());

    std::copy(view.data(), view.data() + view.size(), &this->buffer_[olen]);
}

ByteArray::ByteArray(ByteArray &&other) noexcept : buffer_(other.buffer_) {}

ByteArray::ByteArray(const ByteArray &other) : buffer_(other.buffer_) {}


ByteArrayView::ByteArrayView(const ByteArray &array) : pointer_(array.data()), len_(array.size()) {}

ByteArrayView::ByteArrayView(const uint8_t *data, uint64_t len) : pointer_(data), len_(len) {}

ByteArray ByteArrayView::toByteArray() { return ByteArray(this->pointer_, this->len_); }

ByteArrayView ByteArrayView::sliced(uint64_t start, uint64_t end) { return {this->pointer_ + start, end - start}; }

ByteArrayView ByteArrayView::sliced(uint64_t start) { return {this->pointer_ + start, this->len_ - start}; }

} // namespace wutils
