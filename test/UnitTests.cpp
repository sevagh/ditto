#include "CircularBuffer.h"
#include "gcem.hpp"
#include "stompbox/MathNeon.h"
#include "stompbox/MiscUtil.h"
#include "stompbox/Window.h"
#include <cmath>
#include <gtest/gtest.h>

TEST(StompboxMiscUtilTests, PowerOfTwo)
{
	EXPECT_FALSE(stompbox::detail::misc_util::is_power_of_two(0));
	EXPECT_TRUE(stompbox::detail::misc_util::is_power_of_two(2));
	EXPECT_FALSE(stompbox::detail::misc_util::is_power_of_two(3));
	EXPECT_TRUE(stompbox::detail::misc_util::is_power_of_two(512));
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
	auto window = stompbox::window::HammingWindow<10>;

	// numpy.hamming(10):
	std::array<float, 10> expected{
	    0.08,       0.18761956, 0.46012184, 0.77,       0.97225861,
	    0.97225861, 0.77,       0.46012184, 0.18761956, 0.08};

	for (size_t i = 0; i < 10; ++i)
		EXPECT_NEAR(window.data[i], expected[i], 0.0001);
}

TEST(StompboxWindow, Hanning)
{
	auto window = stompbox::window::HanningWindow<10>;

	// numpy.hanning(10):
	std::array<float, 10> expected{
	    0.,         0.11697778, 0.41317591, 0.75,       0.96984631,
	    0.96984631, 0.75,       0.41317591, 0.11697778, 0.};

	for (size_t i = 0; i < 10; ++i)
		EXPECT_NEAR(window.data[i], expected[i], 0.0001);
}

TEST(StompboxWindow, Blackman)
{
	auto window = stompbox::window::BlackmanWindow<10>;

	// numpy.blackman(10):
	std::array<float, 10> expected{
	    -1.38777878e-17, 5.08696327e-02, 2.58000502e-01, 6.30000000e-01,
	    9.51129866e-01,  9.51129866e-01, 6.30000000e-01, 2.58000502e-01,
	    5.08696327e-02,  -1.38777878e-17};

	for (size_t i = 0; i < 10; ++i)
		EXPECT_NEAR(window.data[i], expected[i], 0.0001);
}

TEST(StompboxWindow, Tukey)
{
	auto window = stompbox::window::TukeyWindow<10>;

	// scipy.signal.windows.tukey(10):
	std::array<float, 10> expected{0., 0.41317591, 0.96984631, 1.,         1.,
	                               1., 1.,         0.96984631, 0.41317591, 0.};

	for (size_t i = 0; i < 10; ++i)
		EXPECT_NEAR(window.data[i], expected[i], 0.0001);
}

TEST(StompboxWindow, Rectangular)
{
	auto window = stompbox::window::RectangularWindow<10>;

	std::array<float, 10> expected{};
	std::fill(expected.begin(), expected.end(), 1.0F);

	for (size_t i = 0; i < 10; ++i)
		EXPECT_NEAR(window.data[i], expected[i], 0.0001);
}
