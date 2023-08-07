#ifndef BUFFER_H
#define BUFFER_H

#include <cstdint>
#include <functional>

namespace wutils {

class IBuffer {
public:
    IBuffer()          = default;
    virtual ~IBuffer() = default;

public:
    virtual bool Init(uint64_t max_buffer_byte) = 0;
    virtual void Release()                      = 0;

    virtual bool IsFull() const  = 0;
    virtual bool IsEmpty() const = 0;

    /**
     * @brief GetReadableBytes
     * @return 可读区大小
     */
    virtual uint64_t GetReadableBytes() const  = 0;
    /**
     * @brief GetWriteableBytes
     * @return 可写区大小
     */
    virtual uint64_t GetWriteableBytes() const = 0;

    /**
     * @brief Size
     * @return 已有数据长度
     */
    virtual uint64_t UsedBytes() const = 0;
    /**
     * @brief MaxSize
     * @return 最大存储长度
     */
    virtual uint64_t MaxSize() const   = 0;

    /**
     * @brief PeekRead
     * @return 可读区起始位置
     */
    virtual uint8_t       *PeekRead()             = 0;
    /**
     * @brief PeekWrite
     * @return 可写区起始位置
     */
    virtual uint8_t       *PeekWrite()            = 0;
    /**
     * @brief ConstPeekRead
     * @return 可读区起始位置
     */
    virtual const uint8_t *ConstPeekRead() const  = 0;
    /**
     * @brief ConstPeekWrite
     * @return 可写区起始位置
     */
    virtual const uint8_t *ConstPeekWrite() const = 0;

    /**
     * @brief SkipReadBytes 跳过可读区
     * @param bytes 跳过字节数
     */
    virtual void SkipReadBytes(uint64_t bytes)    = 0;
    /**
     * @brief SkipAllReadBytes
     */
    virtual void SkipAllReadBytes()               = 0;
    /**
     * @brief UpdateWriteBytes
     * @param bytes
     */
    virtual void UpdateWriteBytes(uint64_t bytes) = 0;

    /**
     * @brief WriteSome 全部写入，不能全部写入时（内部存储大小不足）返回失败，否则成功。
     * @param data
     * @param bytes
     * @return
     */
    virtual bool     Write(const uint8_t *data, uint64_t bytes)     = 0;
    /**
     * @brief ReadSome 全部读出，不能取出全部数据时（外部存储大小不足）返回失败，否则成功。
     * @param buffer
     * @param buffer_len
     * @return
     */
    virtual bool     Read(uint8_t *buffer, uint64_t buffer_len)     = 0;
    /**
     * @brief WriteSome 尽可能写入，返回写入数据长度
     * @param data
     * @param bytes
     * @return
     */
    virtual uint64_t WriteSome(const uint8_t *data, uint64_t bytes) = 0;
    /**
     * @brief ReadSome 尽可能写入，返回读取数据长度
     * @param buffer
     * @param buffer_len
     * @return
     */
    virtual uint64_t ReadSome(uint8_t *buffer, uint64_t buffer_len) = 0;

#if 0
    /**
     * @brief 写入回调
     * @param data
     * @param bytes
     * @return 写入长度
     */
    using writecb = std::function<uint64_t(uint8_t *buffer, uint64_t buffer_len)>;

    /**
     * @brief WriteUntil
     * @param cb
     */
    virtual void WriteUntil(writecb cb) = 0;

    /**
     * @brief 读出回调
     * @param buffer
     * @param buffer_len
     * @return 读出长度
     */
    using readcb = std::function<uint64_t(const uint8_t *data, uint64_t bytes)>;

    /**
     * @brief ReadUntil
     * @param cb
     */
    virtual void ReadUntil(readcb cb) = 0;
#endif
};

} // namespace wutils

#include "buffer/ChainBuffer.h"
#include "buffer/RingBuffer.h"
#include "buffer/StraightBuffer.h"

#endif // BUFFER_H
