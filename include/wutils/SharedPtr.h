#pragma once
#ifndef UTILS_SHARED_PTR_H
#define UTILS_SHARED_PTR_H

#include <memory>

namespace wutils {

template <typename T>
using SharedPtr = std::shared_ptr<T>;

}

#endif // UTILS_SHARED_PTR_H
