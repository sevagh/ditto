#ifndef STOMPBOX_WINDOW_H
#define STOMPBOX_WINDOW_H

#include "MathNeon.h"
#include "gcem.hpp"
#include <array>
#include <cstddef>

namespace stompbox::window {

template <std::size_t WindowSize>
struct Window {
	float data[WindowSize];
};

namespace detail {
	template <std::size_t WindowSize>
	constexpr Window<WindowSize> calculate_hamming_window()
	{
		auto N = ( float )(WindowSize - 1);
		float n_val = 0.0F;

		Window<WindowSize> window = {};
		for (size_t n = 0; n < WindowSize; ++n) {
			window.data[n] = 0.54F
			                 - (0.46F
			                    * gcem::cos(2.0F * stompbox::math_neon::PI
			                                * ((n_val) / N)));
			n_val += 1.0F;
		}
		return window;
	}

	template <std::size_t WindowSize>
	constexpr Window<WindowSize> calculate_hanning_window()
	{
		auto N = ( float )(WindowSize - 1);

		Window<WindowSize> window = {};
		for (size_t n = 0; n < WindowSize; ++n) {
			window.data[n] = 0.5F
			                 * (1.0F
			                    - gcem::cos(2.0F * stompbox::math_neon::PI
			                                * (n/N)));
		}
		return window;
	}
	template <std::size_t WindowSize>
	constexpr Window<WindowSize> calculate_blackman_window()
	{
		auto N = ( float )(WindowSize - 1);
		float n_val = 0.0F;

		Window<WindowSize> window = {};
		for (size_t n = 0; n < WindowSize; ++n) {
			{
				window.data[n] = 0.42F
				            - (0.5F
				               * gcem::cos(2.0F * stompbox::math_neon::PI
				                           * (n_val / N)))
				            + (0.08F
				               * gcem::cos(4.0F * stompbox::math_neon::PI
				                           * (n_val / N)));
				n_val = n_val + 1.0F;
			}
		}

		return window;
	}

	template <std::size_t WindowSize>
	constexpr Window<WindowSize> calculate_tukey_window()
	{
		auto N = ( float )(WindowSize - 1);
		auto n_val = ( float )(-1.0F * ((WindowSize / 2))) + 1.0F;
		float alpha = 0.5F;

		auto width = alpha * (N / 2.0F);

		Window<WindowSize> window = {};
		for (size_t n = 0; n < WindowSize; ++n) {
			if ((n_val >= 0.0F) && (n_val <= width)) {
				window.data[n] = 1.0F;
			}
			else if ((n_val <= 0.0F)
			         && (n_val >= (-1.0F * width))) {
				window.data[n] = 1.0F;
			}
			else {
				window.data[n] = 0.5F
				            * (1.0F
				               + gcem::cos(
				                   stompbox::math_neon::PI
				                   * (((2.0F * n_val) / (alpha * N)) - 1.0F)));
			}

			n_val = n_val + 1.0F;
		}

		return window;
	}

	template <std::size_t WindowSize>
	constexpr Window<WindowSize> calculate_rectangular_window()
	{
		Window<WindowSize> window = {};

		for (size_t n = 0; n < WindowSize; ++n) {
			window.data[n] = 1.0;
		}
		return window;
	}

} // namespace detail

template <std::size_t WindowSize>
constexpr Window HammingWindow = detail::calculate_hamming_window<WindowSize>();

template <std::size_t WindowSize>
constexpr Window HanningWindow = detail::calculate_hanning_window<WindowSize>();

template <std::size_t WindowSize>
constexpr Window BlackmanWindow
    = detail::calculate_blackman_window<WindowSize>();

template <std::size_t WindowSize>
constexpr Window TukeyWindow = detail::calculate_tukey_window<WindowSize>();

template <std::size_t WindowSize>
constexpr Window RectangularWindow
    = detail::calculate_rectangular_window<WindowSize>();
} // namespace stompbox::window

#endif // STOMPBOX_WINDOW_H
