#include "BitMap.h"


void BitMap64::Set(uint8_t value) {
    assert(value < 64);
    this->data_ |= 1 << value;
}

void BitMap64::Del(uint8_t value) {
    assert(value < 64);
    this->data_ &= ~(1 << value);
}

bool BitMap64::Get(uint8_t value) const {
    assert(value < 64);
    return this->data_ & (1 << value);
}

BitMap::BitMap(uint64_t size) : size_(size) {
    auto len = size / BitMap64::Size();
    if(size_ % BitMap64::Size()) {
        ++len;
    }

    for(int i = 0; i < len; ++i) {
        this->data_.emplace_back();
    }
}


void BitMap::Set(uint64_t value) {
    assert(value < this->size_);

    this->data_[value / BitMap64::Size()].Set(value % BitMap64::Size());
}

void BitMap::Del(uint64_t value) {
    assert(value < this->size_);

    this->data_[value / BitMap64::Size()].Del(value % BitMap64::Size());
}

bool BitMap::Get(uint64_t value) const {
    assert(value < this->size_);

    return this->data_[value / BitMap64::Size()].Get(value % BitMap64::Size());
}

void BitMap::Full() {
    std::for_each(this->data_.begin(), this->data_.end(), [](BitMap64 &map) { map.Full(); });
}

void BitMap::Clear() {
    std::for_each(this->data_.begin(), this->data_.end(), [](BitMap64 &map) { map.Clear(); });
}
