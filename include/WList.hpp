#pragma once
#ifndef UTILS_WLIST_H
#define UTILS_WLIST_H

#include <new>
#include <cstdint>
#include "WDebugger.hpp"

namespace wlb {

// 链表
// 这是一个比较“神奇”的链表，用户需要手动创建节点，
// 用户销毁节点，节点将自动从母链表删除，
// 节点仅有一个母链表
// 用户将节点加入到另一个链表时，默认将失败，也提供了强制加入的方法
// 
// not thread safe
template <typename T>
class WList {
public:
    // 链表节点
    class WListNode {
        friend class WList;
    public:
        WListNode() = default;
        explicit WListNode(T value) : val(value) {};
        ~WListNode() { Destroy(); };
        void Destroy();
        T val;
    private:
        WListNode *next{nullptr};
        WListNode *last{nullptr};
        // 仅可入一个链表
        WList     *parent_{nullptr};
    };
private:
    // 链表头
    class WListHead : public WListNode {
    private:
        WListHead *last{nullptr};
    };
    // 链表尾
    class WListTail : public WListNode {
    private:
        WListHead *next{nullptr};
    };

public:
    WList() = default;
    ~WList() { this->Destroy(); }

    bool Init();
    void clear();
    void Destroy();

    inline WListNode *front() { return this->head->next; }
    inline WListNode *back() { return this->tail->last; }

    inline uint32_t size() { return this->_count; }
    inline bool empty() { return this->_count == 0; }

    void push_back(WListNode *node);
    void push_front(WListNode *node);

    void strong_push_back(WListNode *node);
    void strong_push_front(WListNode *node);

    void pop_back();
    void pop_front();

    void erase(WListNode *node);

    bool has_node(WListNode *node);

private:
    void DestroyAllNodes();

private:
    WListHead *head{nullptr};
    WListTail *tail{nullptr};
    uint32_t  _count{0};
};

template <typename T>
void WList<T>::WListNode::Destroy() {
    if (this->parent_ != nullptr) {
        this->parent_->erase(this);
    }
}

template <typename T>
bool WList<T>::Init() {
    this->head = new(std::nothrow) WListHead();
    this->tail = new(std::nothrow) WListTail();
    if (this->head == nullptr || this->tail == nullptr) {
        return false;
    }

    this->head->next = this->tail;
    this->tail->last = this->head;
    return true;
}

template <typename T>
void WList<T>::clear() {
    this->DestroyAllNodes();
    this->_count = 0;
}

template <typename T>
void WList<T>::Destroy() {
    this->clear();
    if (this->head != nullptr) {
        delete this->head;
        this->head = nullptr;
    }
    if (this->tail != nullptr) {
        delete this->tail;
        this->tail = nullptr;
    }
}

template <typename T>
void WList<T>::push_back(WListNode *node) {
    if (node->parent_ != nullptr) {
        return;
    }

    this->tail->last->next = node;
    node->last             = this->tail->last;
    node->next             = this->tail;
    this->tail->last       = node;
    this->_count++;

    node->parent_ = this;
}

template <typename T>
void WList<T>::push_front(WListNode *node) {
    if (node->parent_ != nullptr) {
        return;
    }

    this->head->next->last = node;
    node->next             = this->head->next;
    node->last             = this->head;
    this->head->next       = node;
    this->_count++;

    node->parent_ = this;
}

template <typename T>
void WList<T>::strong_push_back(WListNode *node) {
    if (node->parent_ != nullptr) {
        node->parent_->erase(node);
    }
    this->push_back(node);
}

template <typename T>
void WList<T>::strong_push_front(WListNode *node) {
    if (node->parent_ != nullptr) {
        node->parent_->erase(node);
    }
    this->push_front(node);
}

template <typename T>
void WList<T>::pop_back() {
    if (this->_count == 0) {
        return;
    }
    this->erase(this->tail->last);
}

template <typename T>
void WList<T>::pop_front() {
    if (this->_count == 0) {
        return;
    }
    this->erase(this->head->next);
}

template <typename T>
void WList<T>::erase(WListNode *node) {
    if (node->parent_ != this) {
        return;
    }

    node->parent_ = nullptr;

    node->last->next = node->next;
    node->next->last = node->last;

    node->next = nullptr;
    node->last = nullptr;

    this->_count--;
}

template <typename T>
bool WList<T>::has_node(WListNode *node) {
    return node->parent_ == this;
}

template <typename T>
void WList<T>::DestroyAllNodes() {
    if (this->_count != 0) {
        for (WListNode *node = this->head->next;
             node != this->tail;
                ) {
            WListNode *temp = node->next;
            delete node;
            node = temp;
        }
    }
};

}   // namespace wlb

#endif // UTILS_WLIST_H

