#ifndef DEFER_H
#define DEFER_H

#include <functional>

namespace wutils {

class Helper {
public:
    Helper(std::function<void()> f) : f_(f){};
    ~Helper() { f_(); }
    std::function<void()> f_;
};

#define DEFER(func)                                                                                                    \
    do {                                                                                                               \
        std::make_shared<wutils::Helper>(func);                                                                        \
    } while(0)

} // namespace wutils

#endif // DEFER_H
