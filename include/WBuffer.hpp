#include <cstdint>
#include <cstring>
#include <string>
#include "WOS.h"

namespace wlb {

// 环形内存
// not thread safe
class RingBuffer {
public:
    explicit RingBuffer();
    ~RingBuffer();
    // unablecopy
    RingBuffer(const RingBuffer &)            = delete;
    RingBuffer &operator=(const RingBuffer &) = delete;

    // class life time
    bool Init(uint32_t maxBufferSize);
    void Clear();
    void Destroy();

    // class methods
    uint32_t    GetRestBufferSize() const;
    uint32_t    GetTopRestBufferSize() const;
    char       *GetRestBuffer(); // 返回写指针指向的位置
    void        UpdateWriteOffset(uint32_t len);
    void        UpdateReadOffset(uint32_t len);
    inline bool Empty() const { return this->is_empty_; };

    // return 0 when false, otherwise return the number of success bytes
    uint32_t InsertMessage(const std::string &message);

    uint32_t GetFrontMessageLength() const;
    // you can get messages from buf or return value
    uint32_t GetFrontMessage(std::string *message, uint32_t len);
    uint32_t GetAllMessage(std::string *message);

protected:
    // buffer
    char    *buffer_{nullptr};
    uint32_t max_buffer_size_{0};

    // offset
    uint32_t read_offset_{0};
    uint32_t write_offset_{0};

    // state
    bool is_full_{false};
    bool is_empty_{true};
};

} // namespace wlb
