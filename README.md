# ditto

Ditto is named after the TC Electronics Ditto looper pedal.

It's an Android version of the magic ringbuffer, intended for use in low-latency audio apps using Oboe.

### Build and test

ditto has only been tested an Android SDK (>= 24) and NDK (>= r20) installed at `$HOME/Android` on my Fedora 31 laptop. Test and bench binaries are built, which can possibly be run with `adb_test.sh` and `adb_bench.sh` depending on your device (works on my Nokia 6.1).
