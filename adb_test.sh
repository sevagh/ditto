#!/usr/bin/env bash

adb push cmake-build-debug-android-ndk/dittotest /data/local/tmp
adb shell chmod +x /data/local/tmp/dittotest
adb shell /data/local/tmp/dittotest
