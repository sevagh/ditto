# stompbox

stompbox is my opinionated, personal library of audio, music, and DSP utils for Android devices.

The goal of stompbox is to house a coherent collection of audio DSP tools specifically optimized for ARMv8 Aarch64 processors on modern Android devices using NEON SIMD instructions.

### Components

* Common math/array/vector operations with NEON
* Circular buffer
* HPSS (Harmonic-Percussive Source Separation)
* STFT/iSTFT
* Beat tracking and tempo estimation
* Onset detection
* Window functions

### Build and test

stompbox's CMakeLists.txt file has only been tested an Android SDK (>= 24) and NDK (>= r20) installed at `$HOME/Android` on my Fedora 31 laptop. A test binary is built, which can possibly be run with `adb_test.sh` depending on your device (works on my Nokia 6.1).
