#pragma once
#ifndef UTILS_BYTEARRAY_H
#define UTILS_BYTEARRAY_H

#include <vector>
#include <string>

namespace wutils {

using ByteArray = std::vector<uint8_t>;

using ustring      = std::basic_string<uint8_t>;
using ustring_view = std::basic_string_view<uint8_t>;

} // namespace wutils

#endif // UTILS_BYTEARRAY_H
