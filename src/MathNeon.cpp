#include "stompbox/MathNeon.h"
#include <arm_neon.h>
#include <cfloat>
#include <cmath>

void stompbox::math_neon::Complex2Magnitude(std::vector<ne10_fft_cpx_float32_t>& complex,
                                  std::vector<float>& magnitude)
{
	// convert the complex vector to a vector of real magnitudes
	float32x4_t sum;

	for (size_t i = 0; i < complex.size(); i += 4) {
		sum = {complex[i].r * complex[i].r + complex[i].i * complex[i].i,
		       complex[i + 1].r * complex[i + 1].r
		           + complex[i + 1].i * complex[i + 1].i,
		       complex[i + 2].r * complex[i + 2].r
		           + complex[i + 2].i * complex[i + 2].i,
		       complex[i + 3].r * complex[i + 3].r
		           + complex[i + 3].i * complex[i + 3].i};

		sum = vrsqrteq_f32(sum);

		vst1q_f32(magnitude.data() + i, sum);
	}
}

void stompbox::math_neon::NormalizeByMax(std::vector<float>& vals)
{
	float32x4_t load;
	float32_t local_max;
	float max = -FLT_MAX;

	// purposely omit the final 4 elements, last elem of FFT is inf which
	// screws up the normalization
	for (size_t i = 0; i < vals.size() - 4; i += 4) {
		load = vld1q_f32(vals.data() + i);
		local_max = vmaxvq_f32(load);
		if (local_max > max)
			max = local_max;
	}

	// ne10 doesn't support the math module yet for aarch64
	// see: https://github.com/projectNe10/Ne10/issues/207
	// i have to hand-write most of this

	float32x4_t scalar = vdupq_n_f32(1.0f / max);

	for (size_t i = 0; i < vals.size(); i += 4) {
		load = vld1q_f32(vals.data() + i);
		vst1q_f32(vals.data() + i, vmulq_f32(load, scalar));
	}
}

void stompbox::math_neon::ThresholdUnder(std::vector<float>& vals, float threshold_dB)
{
	float32x4_t local;
	float32x4_t scalar = vdupq_n_f32(20.0f);

	for (size_t i = 0; i < vals.size() - 4; i += 4) {
		local = vld1q_f32(vals.data() + i);
		local = math_neon::log10(local);
		vmulq_f32(local, scalar);
		if (vgetq_lane_f32(local, 0) < threshold_dB)
			vals[i] = 0;
		if (vgetq_lane_f32(local, 1) < threshold_dB)
			vals[i + 1] = 0;
		if (vgetq_lane_f32(local, 2) < threshold_dB)
			vals[i + 2] = 0;
		if (vgetq_lane_f32(local, 3) < threshold_dB)
			vals[i + 3] = 0;
	}
}

void stompbox::math_neon::NormalizeArray(float* x, size_t N)
{
    float sum = 0.0F;

    for (size_t i = 0; i < N; i++) {
        if (x[i] > 0) {
            sum += x[i];
        }
    }

    if (sum > 0) {
        for (size_t i = 0; i < N; i++) {
            x[i] /= sum;
        }
    }
};

void stompbox::math_neon::AdaptiveThreshold(float* x, size_t N)
{
    size_t i = 0;
    size_t k = 0;
    size_t t = 0;
    float x_thresh[N];

    size_t p_post = 7;
    size_t p_pre = 8;

    t = std::min(
        N, p_post); // what is smaller, p_post or df size. This is to
    // avoid accessing outside of arrays

    // find threshold for first 't' samples, where a full average cannot be
    // computed yet
    for (i = 0; i <= t; ++i) {
        k = std::min((i + p_pre), N);
        x_thresh[i] = stompbox::math_neon::CalculateMeanOfArray(x, 1, k);
    }
    // find threshold for bulk of samples across a moving average from
    // [i-p_pre,i+p_post]
    for (i = t + 1; i < N - p_post; ++i) {
        x_thresh[i] = stompbox::math_neon::CalculateMeanOfArray(x, i - p_pre, i + p_post);
    }

    // for last few samples calculate threshold, again, not enough samples
    // to do as above
    for (i = N - p_post; i < N; ++i) {
        k = std::max<size_t>(i - p_post, 1);
        x_thresh[i] = stompbox::math_neon::CalculateMeanOfArray(x, k, N);
    }

    // subtract the threshold from the detection function and check that it
    // is not less than 0
    for (i = 0; i < N; ++i) {
        x[i] = x[i] - x_thresh[i];
        if (x[i] < 0) {
            x[i] = 0;
        }
    }
};

float stompbox::math_neon::CalculateMeanOfArray(const float* array, size_t start, size_t end)
{
    float sum = 0;
    size_t length = end - start;

    // find sum
    for (size_t i = start; i < end; i++) {
        sum = sum + array[i];
    }

    return (length > 0) ? sum / length : 0;
};
