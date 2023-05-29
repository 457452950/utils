#ifndef CHAINBUFFER_H
#define CHAINBUFFER_H

#include "wutils/Buffer.h"

#include <list>
#include <memory>

constexpr uint16_t PAGE_SIZE       = UINT16_MAX;             // 64kb 2^16
constexpr uint16_t PAGE_COUNT      = 64;                     // 64   2^6
constexpr uint64_t MAX_BUFFER_SIZE = PAGE_SIZE * PAGE_COUNT; // 4mb  2^22

// TODO:增加上限检查
class ChainBuffer final : public Buffer {
public:
    ChainBuffer();
    ~ChainBuffer() override = default;

    ChainBuffer(const ChainBuffer &other);
    ChainBuffer(const ChainBuffer &&other) noexcept;
    ChainBuffer &operator=(const ChainBuffer &other);

public:
    bool Init(uint64_t) override;
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
    void check();

private:
    /**
     * [===========================================]
     * |        |               |                  |
     * 0      r_offset       w_offset             max
     *
     * readable  area: [r_offset, w_offset)
     * writeable area: [w_offset, max)
     */
    struct Page {
        std::vector<uint8_t> start;
        uint16_t             r_offset;
        uint16_t             w_offset;
        Page() : start(PAGE_SIZE, 0), r_offset(0), w_offset(0) {}
    };
    std::list<Page> pages_;
};

#endif // CHAINBUFFER_H
