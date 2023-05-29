#ifndef STD_IOVEC_H
#define STD_IOVEC_H

#include <algorithm>
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
    void Release();
    bool IsFull();
    bool IsEmpty();

    using read_loop_cb_t = std::function<int64_t(const uint8_t *, uint32_t buf_len)>;
    using read_cb_t      = std::function<int64_t(const iovec *, uint32_t)>;
    // 从 IOVec 中读取数据
    int64_t Read(read_cb_t cb);
    int64_t Read(read_loop_cb_t cb);

    using write_loop_cb_t = std::function<int64_t(uint8_t *, uint32_t buf_len)>;
    using write_cb_t      = std::function<int64_t(iovec *, uint32_t)>;
    // 往 IOVec 中写入数据
    int64_t Write(write_cb_t cb);
    int64_t Write(const uint8_t *, uint32_t);

private:
private:
    std::list<iovec *>           data_list_;
    std::list<iovec *>::iterator cur_page_;
    uint32_t                     max_page_count_{0};
    uint32_t                     max_page_size_{0};
};

inline IOVec::IOVec() = default;

inline IOVec::IOVec(int page_count, int page_size) { Init(page_count, page_size); }

inline IOVec::~IOVec() { this->Release(); }

inline void IOVec::Init(int page_count, int page_size) {
    assert(page_count < UIO_MAXIOV);
    assert(page_count != 0);

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

inline void IOVec::Release() {
    std::for_each(this->data_list_.begin(), this->data_list_.end(), [](iovec *it) {
        delete[](uint8_t *)it->iov_base;
        delete it;
    });
    this->data_list_.clear();
    max_page_count_ = 0;
    max_page_size_  = 0;
}

inline bool IOVec::IsFull() {
    return (std::distance(cur_page_, this->data_list_.end()) == 1) && cur_page_.operator*()->iov_len == max_page_size_;
}

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

    //
    auto distance = std::distance(this->data_list_.begin(), cur_page_) + 1;
    // auto temp_iovec = new iovec[distance];
    auto temp_iovec = std::unique_ptr<iovec[]>(new iovec[distance]{0});


    // init temp_iovec
    int  ind = 0;
    auto it  = this->data_list_.begin();
    do {
        temp_iovec[ind].iov_base = it.operator*()->iov_base;
        temp_iovec[ind].iov_len  = it. operator*()->iov_len;

        // std::cout << "len " << temp_iovec[ind].iov_len << std::endl;

        if(it == this->cur_page_) {
            break;
        }

        ++it;
        ++ind;
    } while(true);

    total_size = cb(temp_iovec.get(), distance);

    if(total_size < 0) {
        return -1;
    }

    auto update_size = total_size;

    do {
        auto begin = this->data_list_.begin();
        auto size  = begin.operator*()->iov_len;
        assert(size != 0);

        if(update_size < (int64_t)size) {
            begin.operator*()->iov_len -= update_size;
        } else {
            begin.operator*()->iov_len = 0;

            if(cur_page_ != this->data_list_.begin()) {
                // take begin to the back
                this->data_list_.splice(data_list_.end(), data_list_, data_list_.begin(), ++data_list_.begin());
            }
        }

        update_size -= size;
        // std::cout << update_size << std::endl;
    } while(update_size > 0);


    return total_size;
}

inline int64_t IOVec::Read(read_loop_cb_t cb) {
    int64_t total_size = 0;

    do {
        if(this->IsEmpty()) {
            break;
        }

        auto page = this->data_list_.begin().operator*();

        int64_t len = cb((uint8_t *)page->iov_base, page->iov_len);

        if(len < (int64_t)page->iov_len) {
            page->iov_len -= len;
        } else {
            page->iov_len = 0;

            if(cur_page_ != this->data_list_.begin()) {
                // take begin to the back
                this->data_list_.splice(data_list_.end(), data_list_, data_list_.begin(), ++data_list_.begin());
            }
        }

        if(len == 0) {
            break;
        }

        total_size += len;

    } while(true);

    return total_size;
}

inline int64_t IOVec::Write(write_cb_t cb) {
    int64_t total_size = 0;

    if(this->IsFull()) {
        std::cout << "is full " << std::endl;
        return total_size;
    }

    // std::cout << "is empty " << IsEmpty() << std::endl;

    // make temp_iovec
    auto distance   = std::distance(cur_page_, this->data_list_.end());
    auto temp_iovec = std::unique_ptr<iovec[]>(new iovec[distance]);

    // init temp_iovec
    int ind = 0;
    for(auto it = cur_page_; it != this->data_list_.end(); ++it) {
        temp_iovec[ind].iov_base = (char *)it.operator*()->iov_base + it.operator*()->iov_len;

        temp_iovec[ind].iov_len = this->max_page_size_ - it.operator*()->iov_len;

        using namespace std;
        // cout << "len " << temp_iovec[ind].iov_len << "  ";

        ++ind;
    }

    //
    total_size = cb(temp_iovec.get(), distance);

    if(total_size <= 0) {
        return 0;
    }

    // update iovec.iov_len
    auto update_size = total_size;
    for(; cur_page_ != this->data_list_.end(); ++cur_page_) {
        auto i = this->max_page_size_ - cur_page_.operator*()->iov_len;

        cur_page_.operator*()->iov_len += (update_size > (int64_t)i) ? i : update_size;

        using namespace std;
        // std::cout << "  len " << cur_page_.operator*()->iov_len << "  ";

        update_size -= i;

        if(update_size < 0 || std::distance(cur_page_, this->data_list_.end()) == 1) {
            break;
        }
    }

    return total_size;
}

inline int64_t IOVec::Write(const uint8_t *buf, uint32_t len) {
    int64_t total_size = 0;

    do {
        if(this->IsFull()) {
            break;
        }

        auto page = cur_page_.operator*();

        auto size    = this->max_page_size_ - (int)page->iov_len;
        auto cp_size = std::min(size, len);

        memcpy((uint8_t *)page->iov_base + page->iov_len, buf + total_size, cp_size);
        page->iov_len += cp_size;
        total_size += cp_size;
        len -= cp_size;

        if(cp_size == size && std::distance(cur_page_, this->data_list_.end()) == 1) {
            ++cur_page_;
        } else {
            break;
        }

    } while(true);


    return total_size;
}

#endif // STD_IOVEC_H