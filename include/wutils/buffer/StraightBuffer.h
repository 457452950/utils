#ifndef UTILS_STRAIGHT_BUFFER_H
#define UTILS_STRAIGHT_BUFFER_H


#include <vector>
#include "../Buffer.h"

namespace wutils {

class StraightBuffer final : public IBuffer {
public:
    StraightBuffer();
    ~StraightBuffer() override { this->Release(); }
    StraightBuffer(const StraightBuffer &other);
    StraightBuffer(StraightBuffer &&other) noexcept;
    StraightBuffer &operator=(const StraightBuffer &other);

public:
    bool Init(uint64_t max_buffer_byte) override;
    void Release() override;

    bool IsFull() const override;
    bool IsEmpty() const override;

    uint64_t GetReadableBytes() const override;
    uint64_t GetWriteableBytes() const override;

    uint64_t UsedBytes() const override;
    uint64_t MaxSize() const override;

    uint8_t       *PeekRead() override;
    uint8_t       *PeekWrite() override;
    const uint8_t *ConstPeekRead() const override;
    const uint8_t *ConstPeekWrite() const override;

    void SkipReadBytes(uint64_t bytes) override;
    void SkipAllReadBytes() override;
    void UpdateWriteBytes(uint64_t bytes) override;

    bool Write(const uint8_t *data, uint64_t bytes) override;
    bool Read(uint8_t *buffer, uint64_t buffer_len) override;

    uint64_t WriteSome(const uint8_t *data, uint64_t bytes) override;
    uint64_t ReadSome(uint8_t *buffer, uint64_t buffer_len) override;

    // void WriteUntil(writecb cb) override {
    //     auto len = cb(this->PeekWrite(), this->GetWriteableBytes());
    //     assert(this->offset_ + len <= this->MaxSize());
    //     this->UpdateWriteBytes(len);
    // };
    // void ReadUntil(readcb cb) override{
    //    auto len = cb(this->ConstPeekRead(), this->GetReadableBytes());
    //    assert(len > this->GetReadableBytes());
    //    this->SkipReadBytes(len);
    // };

private:
    ByteArray buffer_;
    uint64_t  offset_{0};
};

} // namespace wutils

#endif // UTILS_STRAIGHT_BUFFER_H
