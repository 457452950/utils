#ifndef UTILS_RING_BUFFER_H
#define UTILS_RING_BUFFER_H

#include "../Buffer.h"
#include "wutils/ByteArray.h"

#include <vector>

namespace wutils {

/**
 * [================================================]
 * |        |               |                       |
 * 0      r_offset       w_offset                   max
 *
 * readable  area: [r_offset, w_offset)
 * writeable area: [w_offset, max)  U  [0, r_offset)
 *
 *
 * [================================================]
 * |        |               |                       |
 * 0      w_offset       r_offset                   max
 *
 * readable  area: [r_offset, max)  U  [0, w_offset)
 * writeable area: [w_offset, r_offset)
 *
 */
class RingBuffer final : public Buffer {
public:
    RingBuffer();
    ~RingBuffer() override = default;
    RingBuffer(const RingBuffer &other);
    RingBuffer(const RingBuffer &&other) noexcept;
    RingBuffer &operator=(const RingBuffer &other);


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
    ByteArray buffer_;
    uint64_t  read_offset_{0};
    uint64_t  write_offset_{0};
    bool      is_full_{false};
};

} // namespace wutils

#endif // UTILS_RING_BUFFER_H
