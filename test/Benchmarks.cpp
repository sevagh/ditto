#include "stompbox/MathNeon.h"
#include <benchmark/benchmark.h>
#include <vector>
#include "arm_neon.h"
#include <numeric>
#include <cmath>

static void BM_MathNeonLog10(benchmark::State& state) {
    std::vector<float> test_floats(1024);
    std::vector<float> log_results(1024);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    float32x4_t load;
    float32x4_t result;

    for (auto _ : state) {
        for (size_t i = 0; i < 1024; i += 4) {
            load = vld1q_f32(test_floats.data() + i);
            result = stompbox::math_neon::log10(load);
            vst1q_f32(log_results.data() + i, result);
        }
    }
}

static void BM_StdLog10(benchmark::State& state) {
    std::vector<float> test_floats(1024);
    std::vector<float> log_results(1024);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);

    for (auto _ : state) {
        std::transform(test_floats.begin(), test_floats.end(),
                       log_results.begin(),
                       [](float x) -> float { return std::log10f(x); });
    }
}

BENCHMARK(BM_MathNeonLog10);
BENCHMARK(BM_StdLog10);
