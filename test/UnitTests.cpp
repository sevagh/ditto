#include <gtest/gtest.h>
#include <Ditto.h>
#include <numeric>

TEST(DittoMagicRingbufferTest, TestReadAndWrite)
{
    auto ringbuf = ditto::MagicRingBuffer(1024);

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

TEST(DittoMagicRingbufferTest, TestLinearReads)
{
    auto ringbuf = ditto::MagicRingBuffer(1024*4);

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

