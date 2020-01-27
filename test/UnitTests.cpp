#include "CircularBuffer.h"
#include "stompbox/MathNeon.h"
#include "stompbox/MiscUtil.h"
#include "stompbox/Window.h"
#include <gtest/gtest.h>
#include <numeric>
#include <stompbox/MagicRingBuffer.h>

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
	auto window = stompbox::window::Hamming<10>;

	// numpy.hamming(10):
	std::array<float, 10> expected{
	    0.08,       0.18761956, 0.46012184, 0.77,       0.97225861,
	    0.97225861, 0.77,       0.46012184, 0.18761956, 0.08};

	for (size_t i = 0; i < 10; ++i)
		EXPECT_NEAR(window.data[i], expected[i], 0.0001);
}

TEST(StompboxWindow, Hanning)
{
	auto window = stompbox::window::Hanning<10>;

	// numpy.hanning(10):
	std::array<float, 10> expected{
	    0.,         0.11697778, 0.41317591, 0.75,       0.96984631,
	    0.96984631, 0.75,       0.41317591, 0.11697778, 0.};

	for (size_t i = 0; i < 10; ++i)
		EXPECT_NEAR(window.data[i], expected[i], 0.0001);
}

TEST(StompboxWindow, Blackman)
{
	auto window = stompbox::window::Blackman<10>;

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
	auto window = stompbox::window::Tukey<10>;

	// scipy.signal.windows.tukey(10):
	std::array<float, 10> expected{0., 0.41317591, 0.96984631, 1.,         1.,
	                               1., 1.,         0.96984631, 0.41317591, 0.};

	for (size_t i = 0; i < 10; ++i)
		EXPECT_NEAR(window.data[i], expected[i], 0.0001);
}

TEST(StompboxWindow, Rectangular)
{
	auto window = stompbox::window::Rectangular<10>;

	std::array<float, 10> expected{};
	std::fill(expected.begin(), expected.end(), 1.0F);

	for (size_t i = 0; i < 10; ++i)
		EXPECT_NEAR(window.data[i], expected[i], 0.0001);
}

TEST(StompboxMagicRingbufferTest, TestReadAndWrite)
{
    auto ringbuf = stompbox::magic_ring_buffer::MagicRingBuffer(1024);

    // fill with 1024 floats
    float *buf = (float*)ringbuf.write_ptr();
    std::iota(buf, buf+1024, 13.37F);
    ringbuf.advance_write_ptr(1024);

    EXPECT_EQ(ringbuf.fill_count(), 1024);

    // read 1024 floats
    float *buf_read = (float*)ringbuf.read_ptr();
    for (size_t i = 0; i < 1024; ++i) {
        EXPECT_NEAR(buf_read[i], 13.37F+i, 0.00001);
    }
    ringbuf.advance_read_ptr(1024);

    EXPECT_EQ(ringbuf.fill_count(), 0);
}

TEST(StompboxMagicRingbufferTest, TestLinearReads)
{
    auto ringbuf = stompbox::magic_ring_buffer::MagicRingBuffer(1024*4);

    float *buf = (float*)ringbuf.write_ptr();

    std::fill(buf, buf+1024, 99.99F);
    ringbuf.advance_write_ptr(1024*4);

    EXPECT_EQ(ringbuf.fill_count(), 1024*4);

    // this might demonstrate the side-by-side memory location advantage
    // of the magic ringbuffer

    // advance the read pointer to the end
    float *buf_read = (float*)ringbuf.read_ptr();
    for (size_t i = 0; i < 1023; ++i) {
        EXPECT_EQ(buf_read[i], 99.99F);
    }
    ringbuf.advance_read_ptr(1023*4);

    buf = (float*)ringbuf.write_ptr();

    std::fill(buf, buf+1024, 33.33F);
    ringbuf.advance_write_ptr(1024*4);

    EXPECT_EQ(ringbuf.fill_count(), 1025*4);

    buf_read = (float*)ringbuf.read_ptr();

    for (size_t i = 0; i < 1024; ++i) {
        EXPECT_EQ(buf_read[i], 33.33F);
    }

    ringbuf.advance_read_ptr(1024*4);

    EXPECT_EQ(ringbuf.fill_count(), 4);
}
