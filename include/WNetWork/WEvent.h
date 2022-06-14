#pragma once

#include <cstdint>

namespace wlb::network {

namespace EventType {
constexpr inline uint8_t EV_Error = 1 << 0;
constexpr inline uint8_t EV_READ  = 1 << 1;
constexpr inline uint8_t EV_WRITE = 1 << 2;
}; // namespace EventType


} // namespace wlb::network
