#pragma once

#include <new>
#include <stdint.h>

namespace wlb
{

// not thread safe
template<typename T>
class WList
{
public:
    struct WListNode {
        friend class WList;
    public:
        WListNode() {}
        WListNode(T value) : val(value) {};
        ~WListNode() {Destroy();};
        T val;
        void Destroy() { 
            if (this->_parent != nullptr)
            {
                this->_parent->erase(this);
            }
        };
    private:
        WListNode* next{nullptr};
        WListNode* last{nullptr};

        // 仅可入一个链表
        WList* _parent{nullptr};
    };
private:
    struct WListHead : public WListNode
    {
    private:
        WListHead* last{nullptr};
    };
    struct WListTail : public WListNode
    {
    private:
        WListHead* next{nullptr};
    };

public:
    WList() {}
    ~WList() { this->Destroy(); }

    bool Init();
    void clear();
    void Destroy();

    inline WListNode* front() { return this->head->next; }
    inline WListNode* back() { return this->tail->last; }

    inline uint32_t size() { return this->_count; }

    void push_back(WListNode* node);
    void push_front(WListNode* node);

    void pop_back();
    void pop_front();

    void erase(WListNode* node);

private:
    void DestroyAllNodes();


private:
    WListHead* head{nullptr};
    WListTail* tail{nullptr};
    uint32_t _count{0};
};

template <typename T>
bool WList<T>::Init()
{
    this->head = new (std::nothrow) WListHead();
    this->tail = new (std::nothrow) WListTail();
    if (this->head == nullptr || this->tail == nullptr)
    {
        return false;
    }

    this->head->next = this->tail;
    this->tail->last = this->head;
    return true;
}

template <typename T>
void WList<T>::clear()
{
    std::cout << "clear" << std::endl;
    this->DestroyAllNodes();
    this->_count = 0;
}

template <typename T> 
void WList<T>::Destroy()
{
    this->clear();
    if (this->head != nullptr)
    {
        delete this->head;
        this->head = nullptr;
    }
    if (this->tail != nullptr)
    {
        delete this->tail;
        this->tail = nullptr;
    }
}

template <typename T>
void WList<T>::push_back(WListNode *node)
{
    if (node->_parent != nullptr)
    {
        return;
    }

    this->tail->last->next = node;
    node->last = this->tail->last;
    node->next = this->tail;
    this->tail->last = node;
    this->_count++;

    node->_parent = this;
}

template <typename T>
void WList<T>::push_front(WListNode *node)
{
    if (node->_parent != nullptr)
    {
        return;
    }

    this->head->next->last = node;
    node->next = this->head->next;
    node->last = this->head;
    this->head->next = node;
    this->_count++;

    node->_parent = this;
}

template <typename T>
void WList<T>::pop_back()
{
    if (this->_count == 0)
    {
        return;
    }
    this->erase(this->tail->last);
}

template <typename T>
void WList<T>::pop_front()
{
    if (this->_count == 0)
    {
        return;
    }
    this->erase(this->head->next);
}

template <typename T>
void WList<T>::erase(WListNode *node)
{
    if (node->_parent != this)
    {
        return;
    }

    node->_parent = nullptr;

    node->last->next = node->next;
    node->next->last = node->last;

    node->next = nullptr;
    node->last = nullptr;

    this->_count--;
}

template <typename T>
void WList<T>::DestroyAllNodes()
{
    if (this->_count != 0)
    {
        for (WListNode *node = this->head->next; 
                node != this->tail; 
                )
        {
            WListNode* temp = node->next;
            delete node;
            node = temp;
        }
    }
};

}   // namespace wlb

