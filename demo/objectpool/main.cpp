#include <iostream>

#include "wutils/object_pool/ObjectPool.h"

using namespace wutils::opool;

class TestClass : public ObjectPool<TestClass>::Object {};

int main(int argc, char **argv) {
    ObjectPool<TestClass>::GetInstance()->Init(50);


    return 0;
}
