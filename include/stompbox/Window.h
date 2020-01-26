#ifndef STOMPBOX_WINDOW_H
#define STOMPBOX_WINDOW_H

#include <cstddef>
#include <array>
#include "MathNeon.h"
#include "gcem.hpp"

namespace stompbox::window {

template<std::size_t WindowSize>
struct Window {
    float data[WindowSize];
};

template<std::size_t WindowSize>
constexpr Window<WindowSize> calculate_hamming_window()
{
    auto N = (float)(WindowSize-1);
    float n_val = 0.0F;

    Window<WindowSize> window = {};
    for (size_t n = 0; n < WindowSize; ++n) {
        window.data[n] = 0.54 - (0.46 * gcem::cos(2.0F * stompbox::math_neon::PI * ((n_val++)/N)));
    }
    return window;
}

template<std::size_t WindowSize>
constexpr Window HammingWindow = calculate_hamming_window<WindowSize>();
}

#endif // STOMPBOX_WINDOW_H
