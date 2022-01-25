#include "WOS.h"
#include <stdint.h>
#include <string>

class RingBuffer
{
public:
    RingBuffer();

    // class life time
    bool Init(uint32_t maxBufferSize);
    void Clear();
    void Destroy();

    // class methods
    inline char* GetBuffer();
    inline void UpdateWriteOffset(uint32_t len);

    // return 0 when false, otherwise return the number of success bytes
    uint32_t InsertMessage(const std::string& message);

    // you can get messages from buf or return value
    inline const std::string& GetFrontMessage(char* buf, uint32_t len);
    

protected:
    char* _buffer;
    uint32_t _maxBufferSize;

    uint32_t _ReadOffset;
    uint32_t _WriteOffset;

    // unablecopy
protected:
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
};



