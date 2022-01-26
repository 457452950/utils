#include "WOS.h"
#include <stdint.h>
#include <string>

namespace wlb
{

class RingBuffer
{
public:
    explicit RingBuffer() = default;
    ~RingBuffer() = default;

    // class life time
    bool Init(uint32_t maxBufferSize);
    void Clear();
    void Destroy();

    // class methods
    const uint32_t GetRestBufferSize();
    char* GetBuffer();
    void UpdateWriteOffset(uint32_t len);
    void UpdateReadOffset(uint32_t len);

    // return 0 when false, otherwise return the number of success bytes
    uint32_t InsertMessage(const std::string& message);

    // you can get messages from buf or return value
    const std::string GetFrontMessage(char* buf, uint32_t len);
    const uint32_t GetAllMessage(std::string& message);
    

protected:
    char* _buffer{nullptr};
    uint32_t _maxBufferSize{0};

    uint32_t _ReadOffset{0};
    uint32_t _WriteOffset{0};

    // unablecopy
protected:
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
};

} // namespace wlb


