#ifndef STD_IOVEC_H
#define STD_IOVEC_H

#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <list>

#include <bits/types/struct_iovec.h>
#include <sys/uio.h>

#include "WNetWorkUtils.h"

////////////////////////////////////////////////////////
//
// for data : "abcdefgh"
// page count:5    page size 5
//
//  "abcde" ---> "fgh" ---> "" ---> "" ---> ""
//
////////////////////////////////////////////////////////
/**
 *
 */
class IOVec {
public:
    explicit IOVec();
    explicit IOVec(int page_count, int page_size);
    ~IOVec();

    void Init(int page_count, int page_size);
    bool IsFull();
    bool IsEmpty();

    using read_loop_cb_t = std::function<int64_t(const uint8_t *, int32_t buf_len)>;
    using read_cb_t      = std::function<int64_t(const iovec *, int)>;
    // 从 IOVec 中读取数据
    int64_t Read(read_cb_t cb);

    using write_loop_cb_t = std::function<int64_t(uint8_t *, int32_t buf_len)>;
    using write_cb_t      = std::function<int64_t(iovec *, int)>;
    // 往 IOVec 中写入数据
    int64_t Write(write_cb_t cb);

private:
private:
    std::list<iovec *>           data_list_;
    std::list<iovec *>::iterator cur_page_;
    int                          max_page_count_{0};
    int                          max_page_size_{0};
};

inline IOVec::IOVec() {}

inline IOVec::IOVec(int page_count, int page_size) { Init(page_count, page_size); }

inline IOVec::~IOVec() {}

inline void IOVec::Init(int page_count, int page_size) {
    assert(page_count < UIO_MAXIOV);

    this->max_page_count_ = page_count;
    this->max_page_size_  = page_size;

    for(size_t i = 0; i < max_page_count_; i++) {
        // clang-format off
        this->data_list_.push_front(new iovec{
            .iov_base = new uint8_t[max_page_size_],
            .iov_len = 0
        });
        // clang-format on
    }

    cur_page_ = data_list_.begin();
}

inline bool IOVec::IsFull() { return (cur_page_ == this->data_list_.end()); }
inline bool IOVec::IsEmpty() {
    // clang-format off
    return 
        (cur_page_ == this->data_list_.begin()) && 
        (this->data_list_.begin().operator*()->iov_len == 0);
    // clang-format on
}

inline int64_t IOVec::Read(read_cb_t cb) {
    int64_t total_size = 0;

    if(this->IsEmpty()) {
        return 0;
    }

    //FIXME: cur_page_ is end
    //
    auto distance   = std::distance(this->data_list_.begin(), cur_page_);
    auto temp_iovec = new iovec[distance + 1];

    // init temp_iovec
    int ind = 0;
    for(auto it = this->data_list_.begin(); it != this->cur_page_; ++it) {
        temp_iovec[ind].iov_base = it.operator*()->iov_base;

        temp_iovec[ind].iov_len = it.operator*()->iov_len;

        ++ind;
    }

    if (cur_page_ != this->data_list_.end())
    {
        temp_iovec[distance].iov_base = cur_page_.operator*()->iov_base;
        temp_iovec[distance].iov_len  = cur_page_. operator*()->iov_len;
        ++distance;
    }

    total_size = cb(temp_iovec, distance);

    auto update_size = total_size;

    do {
        auto begin = this->data_list_.begin();
        auto size  = begin.operator*()->iov_len;
        assert(size != 0);

        if(update_size < size) {
            begin.operator*()->iov_len -= update_size;
        } else {
            begin.operator*()->iov_len = 0;

            if(cur_page_ != data_list_.begin())
                // take begin to the back
                this->data_list_.splice(data_list_.end(), data_list_, data_list_.begin(), ++data_list_.begin());
        }

        update_size -= size;
    } while(update_size > 0);


    return total_size;
}

inline int64_t IOVec::Write(write_cb_t cb) {
    int64_t total_size = 0;

    if(this->IsFull()) {
        return total_size;
    }

    std::cout << "is empty " << IsEmpty() << std::endl;

    //
    auto distance   = std::distance(cur_page_, this->data_list_.end());
    auto temp_iovec = new iovec[distance];

    // init temp_iovec
    int ind = 0;
    for(auto it = cur_page_; it != this->data_list_.end(); ++it) {
        temp_iovec[ind].iov_base = it.operator*()->iov_base + it.operator*()->iov_len;

        temp_iovec[ind].iov_len = this->max_page_size_ - it.operator*()->iov_len;
        using namespace std;
        cout << "len " << temp_iovec[ind].iov_len << "  ";

        ++ind;
    }

    //
    total_size = cb(temp_iovec, distance);

    if(total_size <= 0) {
        return 0;
    }

    // update iovec.iov_len
    auto update_size = total_size;
    for(; cur_page_ != this->data_list_.end(); ++cur_page_) {
        auto i = this->max_page_size_ - cur_page_.operator*()->iov_len;

        cur_page_.operator*()->iov_len += (update_size > i) ? i : update_size;

        update_size -= i;

        if(update_size < 0) {
            break;
        }
    }

    return total_size;
}


#endif // STD_IOVEC_H