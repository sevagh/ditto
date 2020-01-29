#!/usr/bin/env bash

adb push cmake-build-debug-android-ndk/dittobench /data/local/tmp
adb shell chmod +x /data/local/tmp/dittobench
adb shell /data/local/tmp/dittobench
