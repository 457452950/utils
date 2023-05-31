#pragma once
#ifndef UTILS_BYTEARRAY_H
#define UTILS_BYTEARRAY_H

#include <cstdint>
#include <cstring>
#include <vector>

namespace wutils {

class ByteArrayView;

class ByteArray final {
public:
    ByteArray();
    ByteArray(const ByteArray &other);
    ByteArray(ByteArray &&other) noexcept;
    explicit ByteArray(uint64_t len, uint8_t value);
    explicit ByteArray(const uint8_t *data, uint64_t len);
    ~ByteArray() = default;

    ByteArray &operator=(const ByteArray &other) = default;


    auto size() const { return this->buffer_.size(); }
    void resize(uint64_t new_size) { this->buffer_.resize(new_size); }
    void clear() { this->buffer_.clear(); }

    auto data() { return this->buffer_.data(); }
    auto data() const { return this->buffer_.data(); }

    // iterator
    auto begin() { return this->buffer_.begin(); }
    auto end() { return this->buffer_.end(); }

    void append(const uint8_t *data, uint64_t len);
    void append(ByteArrayView view);
    void append(const ByteArray &other);

    ByteArray sliced(uint64_t start, uint64_t end);
    ByteArray sliced(uint64_t start);

private:
    std::vector<uint8_t> buffer_;
};

class ByteArrayView final {
public:
    ~ByteArrayView()                     = default;
    ByteArrayView(const ByteArrayView &) = default;
    ByteArrayView(const ByteArray &array);
    ByteArrayView(const uint8_t *data, uint64_t len);


    ByteArrayView(ByteArrayView &&other) = delete;
    ByteArrayView(ByteArray &&other)     = delete;

    ByteArray toByteArray();

    auto data() const { return this->pointer_; }
    auto size() const { return this->len_; }

    ByteArrayView sliced(uint64_t start, uint64_t end);
    ByteArrayView sliced(uint64_t start);

private:
    const uint8_t *pointer_{nullptr};
    uint64_t       len_;
};


} // namespace wutils

#endif // UTILS_BYTEARRAY_H
