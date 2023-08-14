#ifndef DEFER_H
#define DEFER_H

#include <functional>
#include <utility>

namespace wutils {

class DeferHelper {
public:
    explicit DeferHelper(std::function<void()> f) : f_(std::move(f)){};
    //    explicit DeferHelper(std::function<void()> &&f) : f_(std::move(f)){};
    ~DeferHelper() { f_(); }
    std::function<void()> f_;
};

#define STR_CONTACT(A, B) A##B
#define STR_CONTACT2(A, B) STR_CONTACT(A, B)

/**
 * @example
 * {                                                        \n
 *      int *pint = new int[10];                            \n
 *      DEFER( [pint]()->void{ delete[] pint; } );          \n
 * }
 */
#define DEFER(func) auto STR_CONTACT2(__temp_, __LINE__) = std::make_shared<wutils::DeferHelper>(func)

} // namespace wutils

#endif // DEFER_H