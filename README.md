# ditto

Ditto is named after the TC Electronics Ditto looper pedal. It's an Android version of the [magic ringbuffer](https://fgiesen.wordpress.com/2012/07/21/the-magic-ring-buffer/), intended for use in low-latency audio apps using Oboe.

You can write any data type to it, but make sure you multiply the fill count by `sizeof(type)` as the API uses `char*` pointers, e.g.:

```c++
auto writeptr = (float*)ringbuf.write_ptr();
std::fill(writeptr, writeptr+1024, audio_data[audio_data_offset]);
audio_data_offset = (audio_data_offset + 1024) % 441000;
ringbuf.advance_write_ptr(1024*4);
```

The main difference from a typical Linux implementation is using `/data/local/tmp` in place of `/tmp` on Android.

The code has been adapted from the [libsoundio](https://github.com/andrewrk/libsoundio) library. My benchmark may be flawed, or I've implemented it poorly, but it's not working good. **You probably shouldn't use this code**. At best it's a **good demo** of building a standalone Android C++ library with modern NDK and CMake.

Simply copying samples into a regular `std::vector` gated with a `std::atomic<bool>` is more performant.

### Build and test

ditto has only been tested an Android SDK (>= 24) and NDK (>= r20) installed at `$HOME/Android` on my Fedora 31 laptop. Test and bench binaries are built with `-DDITTO_TESTS=ON` (ON by default).

They can possibly be run with `adb_test.sh` depending on your device (works on my Nokia 6.1), but I'm no adb expert.

Run unit tests:
```
sevagh:ditto $ ./adb_test.sh test
./cmake-build-android-ndk-debug/dittotest: 1 file pushed. 26.0 MB/s (5188688 bytes in 0.190s)
Running main() from /home/sevagh/repos/ditto/thirdparty/googletest/src/gtest_main.cc
[==========] Running 2 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 2 tests from DittoMagicRingbufferTest
[ RUN      ] DittoMagicRingbufferTest.TestReadAndWrite
[       OK ] DittoMagicRingbufferTest.TestReadAndWrite (1 ms)
[ RUN      ] DittoMagicRingbufferTest.TestLinearReads
[       OK ] DittoMagicRingbufferTest.TestLinearReads (0 ms)
[----------] 2 tests from DittoMagicRingbufferTest (1 ms total)

[----------] Global test environment tear-down
[==========] 2 tests from 1 test suite ran. (1 ms total)
[  PASSED  ] 2 tests.
```

Run benchmarks:
```
sevagh:ditto $ ./adb_test.sh bench
./cmake-build-android-ndk-debug/dittobench: 1 file pushed. 31.0 MB/s (5166848 bytes in 0.159s)
2020-01-30 10:37:44
Running /data/local/tmp/dittobench
Run on (8 X 2208 MHz CPU s)
CPU Caches:
  L1 Data 32K (x8)
  L1 Instruction 32K (x8)
  L2 Unified 1024K (x2)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
--------------------------------------------------------------
Benchmark                   Time             CPU   Iterations
--------------------------------------------------------------
BM_MagicRingBuffer   31582282 ns      2190442 ns          100
BM_PlainVector         760314 ns       755264 ns          930
```
