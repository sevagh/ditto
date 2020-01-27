#include "stompbox/MathNeon.h"
#include "stompbox/OnsetDetection.h"
#include <benchmark/benchmark.h>
#include <vector>
#include "arm_neon.h"
#include <numeric>
#include <cmath>

#define FRAME_SIZE 1024
#define HOP_SIZE FRAME_SIZE/2

static void BM_MathNeonLog10(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::vector<float> log_results(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    float32x4_t load;
    float32x4_t result;

    for (auto _ : state) {
        for (size_t i = 0; i < FRAME_SIZE; i += 4) {
            load = vld1q_f32(test_floats.data() + i);
            result = stompbox::math_neon::log10(load);
            vst1q_f32(log_results.data() + i, result);
        }
    }
}

static void BM_StdLog10(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::vector<float> log_results(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);

    for (auto _ : state) {
        std::transform(test_floats.begin(), test_floats.end(),
                       log_results.begin(),
                       [](float x) -> float { return std::log10f(x); });
    }
}

static void BM_OnsetDetectionEnergyEnvelope(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    auto odf = stompbox::onset_detection::OnsetDetectionFunction<FRAME_SIZE, HOP_SIZE>(
        stompbox::onset_detection::OnsetDetectionFunctionType::EnergyEnvelope
        );

    for (auto _ : state) {
        odf.calculate_sample(test_floats);
    }
}

static void BM_OnsetDetectionEnergyDifference(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    auto odf = stompbox::onset_detection::OnsetDetectionFunction<FRAME_SIZE, HOP_SIZE>(
        stompbox::onset_detection::OnsetDetectionFunctionType::EnergyDifference
    );

    for (auto _ : state) {
        odf.calculate_sample(test_floats);
    }
}

static void BM_OnsetDetectionPhaseDeviation(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    auto odf = stompbox::onset_detection::OnsetDetectionFunction<FRAME_SIZE, HOP_SIZE>(
        stompbox::onset_detection::OnsetDetectionFunctionType::PhaseDeviation
    );

    for (auto _ : state) {
        odf.calculate_sample(test_floats);
    }
}

static void BM_OnsetDetectionHighFrequencyContent(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    auto odf = stompbox::onset_detection::OnsetDetectionFunction<FRAME_SIZE, HOP_SIZE>(
        stompbox::onset_detection::OnsetDetectionFunctionType::HighFrequencyContent
    );

    for (auto _ : state) {
        odf.calculate_sample(test_floats);
    }
}

static void BM_OnsetDetectionSpectralDifference(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    auto odf = stompbox::onset_detection::OnsetDetectionFunction<FRAME_SIZE, HOP_SIZE>(
        stompbox::onset_detection::OnsetDetectionFunctionType::SpectralDifference
    );

    for (auto _ : state) {
        odf.calculate_sample(test_floats);
    }
}

static void BM_OnsetDetectionSpectralDifferenceHWR(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    auto odf = stompbox::onset_detection::OnsetDetectionFunction<FRAME_SIZE, HOP_SIZE>(
        stompbox::onset_detection::OnsetDetectionFunctionType::SpectralDifferenceHWR
    );

    for (auto _ : state) {
        odf.calculate_sample(test_floats);
    }
}

static void BM_OnsetDetectionComplexSpectralDifference(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    auto odf = stompbox::onset_detection::OnsetDetectionFunction<FRAME_SIZE, HOP_SIZE>(
        stompbox::onset_detection::OnsetDetectionFunctionType::ComplexSpectralDifference
    );

    for (auto _ : state) {
        odf.calculate_sample(test_floats);
    }
}

static void BM_OnsetDetectionComplexSpectralDifferenceHWR(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    auto odf = stompbox::onset_detection::OnsetDetectionFunction<FRAME_SIZE, HOP_SIZE>(
        stompbox::onset_detection::OnsetDetectionFunctionType::ComplexSpectralDifferenceHWR
    );

    for (auto _ : state) {
        odf.calculate_sample(test_floats);
    }
}
static void BM_OnsetDetectionHighFrequencySpectralDifference(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    auto odf = stompbox::onset_detection::OnsetDetectionFunction<FRAME_SIZE, HOP_SIZE>(
        stompbox::onset_detection::OnsetDetectionFunctionType::HighFrequencySpectralDifference
    );

    for (auto _ : state) {
        odf.calculate_sample(test_floats);
    }
}

static void BM_OnsetDetectionHighFrequencySpectralDifferenceHWR(benchmark::State& state) {
    std::vector<float> test_floats(FRAME_SIZE);
    std::iota(test_floats.begin(), test_floats.end(), 0.0F);
    auto odf = stompbox::onset_detection::OnsetDetectionFunction<FRAME_SIZE, HOP_SIZE>(
        stompbox::onset_detection::OnsetDetectionFunctionType::HighFrequencySpectralDifferenceHWR
    );

    for (auto _ : state) {
        odf.calculate_sample(test_floats);
    }
}

//BENCHMARK(BM_MathNeonLog10);
//BENCHMARK(BM_StdLog10);
BENCHMARK(BM_OnsetDetectionEnergyEnvelope);
BENCHMARK(BM_OnsetDetectionEnergyDifference);
BENCHMARK(BM_OnsetDetectionPhaseDeviation);
BENCHMARK(BM_OnsetDetectionHighFrequencyContent);
BENCHMARK(BM_OnsetDetectionHighFrequencySpectralDifference);
BENCHMARK(BM_OnsetDetectionHighFrequencySpectralDifferenceHWR);
BENCHMARK(BM_OnsetDetectionSpectralDifference);
BENCHMARK(BM_OnsetDetectionSpectralDifferenceHWR);
BENCHMARK(BM_OnsetDetectionComplexSpectralDifference);
BENCHMARK(BM_OnsetDetectionComplexSpectralDifferenceHWR);
