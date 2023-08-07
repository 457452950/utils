#pragma once
#ifndef UTIL_OBJECT_POOL_H
#define UTIL_OBJECT_POOL_H

#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "wutils/base/InstanceHelper.h"

namespace wutils::opool {

enum IncreaseType {
    STEP,
    TIMES,
};

/**
 *
 * @tparam T
 * @example
 *  class Example : public ObjectPool\<Example\>::Object {}; \n
 *  Example* ex1 = new Example(); \n
 *  delete ex1; \n
 *  Example ex2[] = new Example[2]; \n
 *  delete[] ex2;
 */
template <class T>
class ObjectPool {
    Instance(ObjectPool);

public:
    ~ObjectPool() = default;

    class Object {
        void *operator new(size_t size) {
            // return ::operator new(size);

            auto pool = ObjectPool<T>::GetInstance();
            return pool->GetObjects(1);
        }
        void *operator new[](size_t size) {
            // return ::operator new[](size);

            auto pool  = ObjectPool<T>::GetInstance();
            auto count = size / ObjectSize();

            return pool->GetObjects(count);
        }
        void operator delete(void *pVoid) {
            auto pool = ObjectPool<T>::GetInstance();
            pool->BackObject(pVoid);
            // ::operator delete(pVoid);
        }
        void operator delete[](void *pVoid) {
            auto pool = ObjectPool<T>::GetInstance();
            pool->BackObjects(pVoid);
            // ::operator delete[](pVoid);
        }
    };

private:
    ObjectPool() {}

    static int ObjectSize() { return sizeof(T); }

    using Object_type = T;
    using Object_ptr  = T *;

public:
    bool       Init(uint32_t size) { return false; }
    void       SetIncrease(IncreaseType type, double v) {}
    Object_ptr GetObjects(uint32_t count) { return nullptr; }
    void       BackObject(Object_ptr object) {}
    void       BackObjects(Object_ptr object) {}

private:
};

template <class T>
InstanceInit(nullptr, ObjectPool<T>);

template <class T>
ObjectPool<T> *CreateObjectPool(uint32_t init_size = 50) {
    auto pool = ObjectPool<T>::GetInstance();
    pool->Init(init_size);
    return pool;
}

} // namespace wutils::opool

#endif // UTIL_OBJECT_POOL_H
