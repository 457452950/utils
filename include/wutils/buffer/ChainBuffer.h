#ifndef UTILS_CHAIN_BUFFER_H
#define UTILS_CHAIN_BUFFER_H

#include <list>
#include <memory>
#include <cassert>

// #include <bits/types/struct_iovec.h>
// #include <sys/uio.h>

#include "wutils/Buffer.h"
#include "wutils/ByteArray.h"
#include "wutils/SharedPtr.h"


namespace wutils {

class ChainBuffer final : public IBuffer {

    static constexpr uint16_t PAGE_SIZE = UINT16_MAX; // 16kb

public:
    ChainBuffer();
    ~ChainBuffer() override = default;

    ChainBuffer(ChainBuffer &&other) noexcept;

    ChainBuffer(const ChainBuffer &other)            = delete;
    ChainBuffer &operator=(const ChainBuffer &other) = delete;

public:
    bool Init(uint64_t) override;
    void Release() override;

    bool IsFull() const override;
    bool IsEmpty() const override;

    uint64_t GetReadableBytes() const override;
    uint64_t GetWriteableBytes() const override;

    uint64_t UsedBytes() const override { return data_size_; }
    uint64_t MaxSize() const override { return UINT64_MAX; }

    uint8_t       *PeekRead() override;
    uint8_t       *PeekWrite() override;
    const uint8_t *ConstPeekRead() const override;
    const uint8_t *ConstPeekWrite() const override;

    void SkipReadBytes(uint64_t bytes) override;
    void SkipAllReadBytes() override;
    void UpdateWriteBytes(uint64_t bytes) override;

    bool Write(const uint8_t *data, uint64_t bytes) override;
    bool Write(unique_ptr<uint8_t[]> data, uint64_t bytes);
    bool Read(uint8_t *buffer, uint64_t buffer_len) override;

    uint64_t WriteSome(const uint8_t *data, uint64_t bytes) override;
    uint64_t ReadSome(uint8_t *buffer, uint64_t buffer_len) override;

    //    void WriteUntil(writecb cb) {
    //        uint64_t len = 0;
    //        do {
    //            if(this->IsFull()) {
    //                break;
    //            }
    //
    //            len = cb(this->PeekWrite(), this->GetWriteableBytes());
    //            this->UpdateWriteBytes(len);
    //        } while(len);
    //    }
    //    void ReadUntil(readcb cb) {
    //        uint64_t len = 0;
    //        do {
    //            len = cb(this->ConstPeekRead(), this->GetReadableBytes());
    //            this->SkipReadBytes(len);
    //        } while(len || this->IsEmpty());
    //    }
private:
    [[nodiscard]] bool checkLastPageIsFull() const {
        assert(!this->pages_.empty());

        auto page = this->pages_.rbegin();
        if(page->w_offset == page->page_size) {
            return true;
        }

        return false;
    }
    [[nodiscard]] bool checkLastPageIsEmpty() const {
        assert(!this->pages_.empty());

        auto page = this->pages_.rbegin();
        return page->w_offset == 0;
    }

private:
    // [===========================================]
    // |        |               |                  |
    // 0      r_offset       w_offset             max
    /**
     *
     * readable  area: [r_offset, w_offset)
     * writeable area: [w_offset, max)
     */
    struct Page {
        unique_ptr<uint8_t[]> start;
        uint64_t              page_size;
        uint64_t              r_offset;
        uint64_t              w_offset;
        Page() : start(make_unique<uint8_t[]>(PAGE_SIZE)), page_size(PAGE_SIZE), r_offset(0), w_offset(0) {}
        Page(unique_ptr<uint8_t[]> p, uint64_t len) : start(std::move(p)), page_size(len), r_offset(0), w_offset(len) {}
    };
    std::list<Page> pages_;

    uint64_t data_size_{0};
};

} // namespace wutils

#endif // UTILS_CHAIN_BUFFER_H
