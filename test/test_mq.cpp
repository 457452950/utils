#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "wutils/message_queue/MQueue.h"

struct TestInt {
    TestInt(int ii) : i(ii) {}
    TestInt() : i(0) {}
    //    TestInt(TestInt &o) : i(o.i) { std::cout << "copy" << std::endl; }
    //    TestInt(TestInt &&o) : i(o.i) { std::cout << "move" << std::endl; }
    //
    //    TestInt &operator=(const TestInt &o) {
    //        i = o.i;
    //        return *this;
    //    }
    //    TestInt &operator=(const TestInt &&o) {
    //        i = o.i;
    //        return *this;
    //    }

    int i;
};
// using TestQueue         = wutils::MQueue<TestInt, std::list<TestInt>>;
using TestQueue         = wutils::MQueue<TestInt>;
constexpr int max_vl    = 10000000;
constexpr int max_count = 3 * max_vl;

void thread_push(TestQueue *queue) {
    using namespace std::chrono_literals;
    for(int i = 0; i < max_vl; ++i) {
        queue->Push(i);
        //        std::this_thread::sleep_for(10ms);
    }
}

void thread_get(TestQueue *queue) {
    TestInt                value;
    int                    current = 0;
    static std::atomic_int count   = 0;
    while(true) {
        auto res = queue->Get(&value);
        if(res == nullptr) {
            //            GTEST_LOG_(INFO) << "nullptr " << count;
        } else {
            //            GTEST_LOG_(INFO) << "get " << value.i;
            //            ASSERT_EQ(current++, value.i);
            count.fetch_add(1);
        }
        if(count.load() == max_count) {
            break;
        }
    }
}

TEST(mq, mqq) {
    TestQueue queue;
    using namespace std::chrono;

    auto time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    std::thread th1(thread_push, &queue);
    std::thread th3(thread_push, &queue);
    std::thread th5(thread_push, &queue);
    std::thread th2(thread_get, &queue);
    //    std::thread th4(thread_get, &queue);


    th1.join();
    th2.join();
    th3.join();
    //    th4.join();
    th5.join();

    GTEST_LOG_(INFO) << duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() - time;

    //    ASSERT_EQ(queue.Size(), max_count);
}
