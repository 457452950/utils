#pragma once

#ifndef _UTILS_BIG_INTEGER_H
#define _UTILS_BIG_INTEGER_H

#include <iostream>
#include <cstdint>

#ifdef OS_IS_LINUX
#include <assert.h>
#endif

class BigInteger {
public:
    explicit BigInteger(uint64_t number, uint16_t len) : len_(0), max_len_(len) {
        assert(len != 0);

        data_ = new uint32_t[max_len_] {0};

        this->Set(number);
    }
    explicit BigInteger(uint16_t len) : len_(0), max_len_(len) {
        assert(len != 0);

        data_ = new uint32_t[max_len_] {0};
    }
    BigInteger(const BigInteger& other) : len_(other.len_), max_len_(other.max_len_) {
        assert(this == &other);
        
        data_ = new uint32_t[max_len_] {0};

        for (uint8_t i = 0; i < len_; i++) {
            this->data_[i] = other.data_[i];
        }

    }
    BigInteger(BigInteger&& other) : len_(other.len_), max_len_(other.max_len_) {
        this->data_ = other.data_;
        other.data_ = nullptr;
    }
    ~BigInteger() {
        delete[] data_;
    }
private:
    /********************************************
     * 存储方式: 0x12345678  
     * data_[0]: 0x5678  ---> data_[1]: 0x1234
     * 
     ********************************************/

    /**
     * 0xFFFF
     * 32 bit(0~4,294,967,295), 存储 9位 10进制数，即最大为 4,294,967,295
     * 64 bit(0~18,446,744,073,709,551,615)
     *          18,446,744,065,119,617,025 = (4,294,967,295)^2
    */
    uint32_t        *data_{nullptr};
    uint8_t         len_{0};
    uint8_t         max_len_{0};

public:
    void    Dump() const;
    bool    IsZero() const;
    int8_t  Compare(const BigInteger& other) const;

    void    Set(uint64_t num);
};

void BigInteger::Dump() const {
    int i = this->len_;

    printf("0x%x", this->data_[i--]);

    for (; i >= 0; --i) {
        printf("'%04x", this->data_[i]);
    }
    
}

bool BigInteger::IsZero() const {
    for (uint8_t i = 0; i < this->len_; ++i) {
        if (this->data_[i] != 0) {
            return false;
        }
    }
    return true;
}

int8_t BigInteger::Compare(const BigInteger& other) const {
    if (this->len_ != other.len_) {
        return this->len_ > other.len_ ? 1 : -1; 
    }

    for (uint8_t i = 0; i < this->len_; i++) {
        if (this->data_[i] != other.data_[i]) {
            return this->data_[i] > other.data_[i] ? 1 : -1; 
        }
        
    }
    
    return 0;
}

void BigInteger::Set(uint64_t num) {
    if (num > 0xFFFF) {
        assert(max_len_ > 1);

        this->len_ = 1;
        this->data_[0] = (num & 0xFFFF);
        this->data_[1] = (num >> 16);
        return;
    }

    this->len_ = 0;
    this->data_[0] = num; 
}

#endif