#ifndef DEFER_H
#define DEFER_H

#include <functional>

class Helper {
public:
    Helper(std::function<void()> f) : f_(f){};
    ~Helper() { f_(); }
    std::function<void()> f_;
};

#define DEFER(func)                                                                                                    \
    do {                                                                                                               \
        std::make_shared<Helper>(func);                                                                                \
    } while(0)

#endif // DEFER_H
