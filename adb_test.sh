#!/usr/bin/env bash

adb push cmake-build-debug-android-ndk/stompboxtest /data/local/tmp
adb push cmake-build-debug-android-ndk/stompboxbench /data/local/tmp
adb shell chmod +x /data/local/tmp/stompboxtest
adb shell chmod +x /data/local/tmp/stompboxbench
adb shell /data/local/tmp/stompboxtest
adb shell /data/local/tmp/stompboxbench
