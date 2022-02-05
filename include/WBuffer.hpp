#include "WOS.h"
#include <stdint.h>
#include <string>
#include <cstring>

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
    const uint32_t GetTopRestBufferSize();
    char* GetBuffer();
    char* GetRestBuffer();
    void UpdateWriteOffset(uint32_t len);
    void UpdateReadOffset(uint32_t len);
    inline bool Empty() { return this->_isEmpty; };

    // return 0 when false, otherwise return the number of success bytes
    uint32_t InsertMessage(const std::string& message);

    // you can get messages from buf or return value
    const uint32_t GetFrontMessage(std::string& message, uint32_t len);
    const uint32_t GetAllMessage(std::string& message);
    

protected:
    char* _buffer{nullptr};
    uint32_t _maxBufferSize{0};

    uint32_t _ReadOffset{0};
    uint32_t _WriteOffset{0};

    bool _isFull{false};
    bool _isEmpty{true};

    // unablecopy
protected:
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
};

} // namespace wlb


