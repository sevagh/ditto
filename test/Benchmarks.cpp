#include "Ditto.h"
#include <benchmark/benchmark.h>

static void BM_MagicRingBuffer(benchmark::State& state) {
    int x = 0;
    for (auto _ : state) {
        x += 5;
    }
}

BENCHMARK(BM_MagicRingBuffer);
