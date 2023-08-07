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
class RingBuffer final : public IBuffer {
public:
    RingBuffer();
    ~RingBuffer() override = default;
    RingBuffer(const RingBuffer &other);
    RingBuffer(RingBuffer &&other) noexcept;
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

    bool Write(const uint8_t *data, uint64_t bytes) override;
    bool Read(uint8_t *buffer, uint64_t buffer_len) override;

    uint64_t WriteSome(const uint8_t *data, uint64_t bytes) override;
    uint64_t ReadSome(uint8_t *buffer, uint64_t buffer_len) override;

    //    void WriteUntil(writecb cb) {
    //        uint64_t len = 0;
    //
    //        do {
    //            len = cb(this->PeekWrite(), this->GetWriteableBytes());
    //            this->UpdateWriteBytes(len);
    //        } while(!this->IsFull() && len != 0);
    //    }
    //
    //    void ReadUntil(readcb cb) {
    //        uint64_t len = 0;
    //
    //        do {
    //            len = cb(this->ConstPeekRead(), this->GetReadableBytes());
    //            this->SkipReadBytes(len);
    //        } while(!this->IsEmpty() && len != 0);
    //    }

private:
    ByteArray buffer_;
    uint64_t  read_offset_{0};
    uint64_t  write_offset_{0};
    bool      is_full_{false};
};

} // namespace wutils

#endif // UTILS_RING_BUFFER_H
