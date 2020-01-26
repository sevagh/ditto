#ifndef STOMPBOX_MATHNEON_H
#define STOMPBOX_MATHNEON_H

#include "NE10.h"
#include <arm_neon.h>
#include <vector>

namespace stompbox::math_neon {
	constexpr float PI = 3.14159265359f;

	void Complex2Magnitude(std::vector<ne10_fft_cpx_float32_t>&,
	                       std::vector<float>&);

	void NormalizeByMax(std::vector<float>&);

	void ThresholdUnder(std::vector<float>&, float);

	float32x4_t log10(float32x4_t);
} // namespace stompbox::math_neon

#endif // STOMPBOX_MATHNEON_H
