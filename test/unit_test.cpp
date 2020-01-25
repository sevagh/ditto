#include <gtest/gtest.h>
#include "MathNeon.h"

TEST(StompboxMathNeonTests, Pi)
{
EXPECT_NEAR(stompbox::math_neon::PI, 3.14159, 0.1);
}
