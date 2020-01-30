#include "Ditto.h"
#include <benchmark/benchmark.h>
#include <chrono>
#include <thread>
#include <numeric>
#include <atomic>

// "10 seconds" of fake audio data at 44.1kHz sampling rate
static float audio_data[441000];
static std::size_t audio_data_offset;
static std::atomic<bool> stop_mrb = false;
static std::atomic<bool> stop_vec = false;

// fill 1024 samples at a time - some plausible DSP algorithm might need to work on 1024 samples
static void write1024_MRB(ditto::MagicRingBuffer &ringbuf) {
    auto writeptr = (float*)ringbuf.write_ptr();
    std::fill(writeptr, writeptr+1024, audio_data[audio_data_offset]);
    audio_data_offset = (audio_data_offset + 1024) % 441000;
    ringbuf.advance_write_ptr(1024*4);
}

// read 1024 samples at a time
static void reader_thread_MRB(ditto::MagicRingBuffer &ringbuf) {
    while (!stop_mrb) {
        if (ringbuf.fill_count() < (1024*4)) {
			continue;
		}

        auto readptr = (float*)ringbuf.read_ptr();

        //pretend readptr is a `float[1024]`
        float sum = 0.0F;
        std::for_each(readptr, readptr+1024, [&](float x) { sum += x * x; });

        ringbuf.advance_read_ptr(1024*4);
    }
}

static void BM_MagicRingBuffer(benchmark::State& state) {
    audio_data_offset = 0;
    std::iota(audio_data, audio_data+441000, 0.0);
    auto ringbuf = ditto::MagicRingBuffer(65536);
    stop_mrb = false;
    auto th = std::thread(reader_thread_MRB, std::ref(ringbuf));
    for (auto _ : state) {
        for (size_t i = 0; i < 441000; i += 1024) {
            write1024_MRB(ringbuf);

            // verify read throughput
            while (ringbuf.fill_count() != 0) {
                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
        }
    }
    stop_mrb = true;
    th.join();
}

// fill 1024 samples at a time - some plausible DSP algorithm might need to work on 1024 samples
static void write1024_vec(std::vector<float> &ringbuf, std::atomic<bool> &flag) {
    std::fill(ringbuf.begin(), ringbuf.end(), audio_data[audio_data_offset]);
    audio_data_offset = (audio_data_offset + 1024) % 441000;
    flag = true;
}

// read 1024 samples at a time
static void reader_thread_vec(std::vector<float> &ringbuf, std::atomic<bool> &flag) {
    while (!stop_vec) {
        float sum = 0.0F;
        std::for_each(ringbuf.begin(), ringbuf.end(), [&](float x) { sum += x * x; });
        flag = false;
    }
}

static void BM_PlainVector(benchmark::State& state) {
    audio_data_offset = 0;
    std::iota(audio_data, audio_data+441000, 0.0);
    auto ringbuf = std::vector<float>(1024);
    std::atomic<bool> vec_ringbuf_flag = true;

    stop_vec = false;
    auto th = std::thread(reader_thread_vec, std::ref(ringbuf), std::ref(vec_ringbuf_flag));
    for (auto _ : state) {
        for (size_t i = 0; i < 441000; i += 1024) {
            write1024_vec(ringbuf, vec_ringbuf_flag);

            // verify read throughput
            while (!vec_ringbuf_flag) {
                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
        }
    }
    stop_vec = true;
    th.join();
}

BENCHMARK(BM_MagicRingBuffer);
BENCHMARK(BM_PlainVector);
