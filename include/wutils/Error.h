#pragma once
#ifndef WUTILS_ERROR_H
#define WUTILS_ERROR_H

#include <system_error>
#include <string>

#include "base/HeadOnly.h"

namespace wutils {

using Error = std::error_code;

using std::error_category;
using std::make_error_code;

HEAD_ONLY Error GetGenericError() { return {errno, std::generic_category()}; }


} // namespace wutils

#endif // WUTILS_ERROR_H
