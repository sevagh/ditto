#include "CircularBuffer.h"
#include "stompbox/MathNeon.h"
#include "stompbox/MiscUtil.h"
#include "stompbox/Window.h"
#include <gtest/gtest.h>

TEST(StompboxMiscUtilTests, PowerOfTwo)
{
    EXPECT_FALSE(stompbox::misc_util::is_power_of_two(0));
    EXPECT_TRUE(stompbox::misc_util::is_power_of_two(2));
    EXPECT_FALSE(stompbox::misc_util::is_power_of_two(3));
    EXPECT_TRUE(stompbox::misc_util::is_power_of_two(512));
}

TEST(StompboxMathNeonTests, Pi)
{
EXPECT_NEAR(stompbox::math_neon::PI, 3.14159, 0.1);
}

TEST(StompboxCircularBufferTests, TestReadAndWrite)
{
    auto circbuf = stompbox_private::circular_buffer::CircularBuffer<16>();
    float testval = 13.337;
    circbuf.append(testval);
    EXPECT_EQ(circbuf[15], testval);
}

TEST(StompboxWindow, Hamming)
{
    const auto hammingWindow = stompbox::window::HammingWindow<1024>;
}
