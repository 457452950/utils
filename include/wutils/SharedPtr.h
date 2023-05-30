#pragma once
#ifndef UTILS_SHARED_PTR_H
#define UTILS_SHARED_PTR_H

#include <memory>

namespace wutils {

using std::enable_shared_from_this;
using std::make_shared;
using std::shared_ptr;

using std::weak_ptr;

using std::make_unique;
using std::unique_ptr;


} // namespace wutils

#endif // UTILS_SHARED_PTR_H
