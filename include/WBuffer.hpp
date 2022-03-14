#include "WOS.h"
#include <stdint.h>
#include <string>
#include <cstring>

namespace wlb
{

// not thread safe
class RingBuffer
{
public:
    explicit RingBuffer();
    ~RingBuffer();

    // class life time
    bool Init(uint32_t maxBufferSize);
    void Clear();
    void Destroy();

    // class methods
    const uint32_t GetRestBufferSize();
    const uint32_t GetTopRestBufferSize();
    char* GetBuffer();
    char* GetRestBuffer();  // 返回写指针指向的位置
    void UpdateWriteOffset(uint32_t len);
    void UpdateReadOffset(uint32_t len);
    inline bool Empty() { return this->_isEmpty; };

    // return 0 when false, otherwise return the number of success bytes
    uint32_t InsertMessage(const std::string& message);

    const uint32_t GetFrontMessageLength();
    // you can get messages from buf or return value
    const uint32_t GetFrontMessage(std::string& message, uint32_t len);
    const uint32_t GetAllMessage(std::string& message);
    
    const std::string GetErrorMessage() {
        std::string _t = this->_errorMessage;
        this->_errorMessage.clear();
        return _t;
    };

protected:
    char* _buffer{nullptr};
    uint32_t _maxBufferSize{0};

    uint32_t _ReadOffset{0};
    uint32_t _WriteOffset{0};

    bool _isFull{false};
    bool _isEmpty{true};

    // error
    std::string _errorMessage;

    // unablecopy
protected:
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
};

} // namespace wlb


