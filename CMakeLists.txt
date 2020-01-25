cmake_minimum_required(VERSION 3.10)
project(stompbox)

set(CMAKE_CXX_STANDARD 17)
set(NDK_DIR $(HOME)/Android/Sdk/ndk/20.1.5948944)
set(ANDROID_ABI arm64-v8a)
set(ANDROID_PLATFORM android-21)

add_definitions(-DANDROID_ABI=arm64-v8a)
add_definitions(-DCMAKE_TOOLCHAIN_FILE=$(NDK_DIR)/build/cmake/android.toolchain.cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target aarch64-linux-android24")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target aarch64-linux-android24")

set(NE10_ANDROID_TARGET_ARCH "aarch64")
set(ANDROID_API_LEVEL 24)
set(NE10_DIR ./thirdparty/Ne10)
add_subdirectory(${NE10_DIR} ./thirdparty/Ne10)
include_directories(${NE10_DIR}/inc)

include_directories(src)

add_library(
        stompbox
        src/Stompbox.cpp
        src/MathNeon.cpp
        src/MathNeonLog10.cpp
)

target_link_libraries(
        stompbox
        NE10
)

set(GOOGLETEST_DIR ./thirdparty/googletest)
add_subdirectory(${GOOGLETEST_DIR} ./thirdparty/googletest)
include_directories(${GOOGLETEST_DIR}/include)

set(TESTDEPS stompbox gtest gtest_main)

add_executable(
        stompboxtest
        test/unit_test.cpp
)

target_link_libraries(
        stompboxtest
        ${TESTDEPS}
)
