#ifndef BITMAP_H
#define BITMAP_H

#include <cassert>
#include <cstdint>

#include <algorithm>
#include <vector>

struct BitMap64 {
public:
    static uint8_t Size() { return 64; };
    // 0~63
    void Set(uint8_t value);
    void Del(uint8_t value);
    bool Get(uint8_t value) const;

    void Full() { this->data_ = UINT64_MAX; }
    void Clear() { this->data_ = 0; }

private:
    uint64_t data_{0};
};

class BitMap {
public:
    BitMap(uint64_t size);

    void Size() const;

    void Set(uint64_t value);
    void Del(uint64_t value);
    bool Get(uint64_t value) const;

    void Full();
    void Clear();

private:
    std::vector<BitMap64> data_;
    uint64_t              size_;
};

#endif // BITMAP_H
