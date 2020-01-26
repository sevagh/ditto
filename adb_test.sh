#!/usr/bin/env bash

adb push cmake-build-debug-android-ndk/stompboxtest /data/local/tmp
adb shell chmod +x /data/local/tmp/stompboxtest
adb shell /data/local/tmp/stompboxtest
