#ifndef STRAIGHTBUFFER_H
#define STRAIGHTBUFFER_H

#include "../Buffer.h"

#include <vector>

class StraightBuffer final : public Buffer {
public:
    StraightBuffer();
    ~StraightBuffer() override { this->Release(); }
    StraightBuffer(const StraightBuffer &other);
    StraightBuffer(const StraightBuffer &&other) noexcept;
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

    bool WriteFixBytes(const uint8_t *data, uint64_t bytes) override;
    bool ReadFixBytes(uint8_t *buffer, uint64_t buffer_len) override;

    uint64_t Write(const uint8_t *data, uint64_t bytes) override;
    uint64_t Read(uint8_t *buffer, uint64_t buffer_len) override;

    void WriteUntil(writecb cb) override;
    void ReadUntil(readcb cb) override;

private:
    std::vector<uint8_t> buffer_;
    uint64_t             offset_{0};
};

#endif // STRAIGHTBUFFER_H
