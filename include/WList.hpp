#pragma once

#include <stdint.h>

namespace wlb
{


template<typename T>
class WList
{
public:
    class WListNode {
    public:
        T val;
        WListNode* next{nullptr};
        WListNode* last{nullptr};
        WListNode(T value) : val(value) {};
        ~WListNode() {}
    };
private:
    class WListHead : public WListNode
    {
    private:
        WListHead* last{nullptr};
    };
    class WListTail : public WListNode
    {
    private:
        WListHead* next{nullptr};
    };

public:
    WList() {}
    ~WList() {}

    void DestroyAllNodes() {};

    void push_back(WListNode* node) {
        this->tail.last->next = node;
        node->last = this->tail.last;
        node->next = this->tail;
        this->tail.next;
        _count++;
    }

private:
    WListHead head;
    WListTail tail;
    uint32_t _count{0};
};


}   // namespace wlb

