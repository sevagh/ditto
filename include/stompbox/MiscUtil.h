#ifndef STOMPBOX_MISCUTIL_H
#define STOMPBOX_MISCUTIL_H

#include <cstddef>

namespace stompbox::misc_util {
    constexpr bool is_power_of_two(std::size_t value) {
        return value != 0 && !(value & (value - 1));
    }

    constexpr bool is_half(std::size_t frame_size, std::size_t window_size) {
        return (frame_size/2 == window_size);
    }
} // namespace stompbox::misc_util

#endif // STOMPBOX_MISCUTIL_H
