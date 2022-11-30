#include "../throttled.h"

#include <gtest/gtest.h>

TEST(THROTTLED, init_and_repeat_delay)
{
    Throttled throttle(100, 50);

    ASSERT_FALSE(throttle(25));
    ASSERT_FALSE(throttle(50));
    ASSERT_FALSE(throttle(75));
    ASSERT_TRUE(throttle(100));

    ASSERT_FALSE(throttle(125));
    ASSERT_TRUE(throttle(150));

    ASSERT_FALSE(throttle(175));
    ASSERT_TRUE(throttle(200));
}

TEST(THROTTLED, reset_before_initial)
{
    Throttled throttle(100, 50);

    ASSERT_FALSE(throttle(25));
    ASSERT_FALSE(throttle(50));
    ASSERT_FALSE(throttle(75));

    ASSERT_FALSE(throttle(25));
    ASSERT_FALSE(throttle(50));
    ASSERT_FALSE(throttle(75));
    ASSERT_TRUE(throttle(100));
}

TEST(THROTTLED, reset_after_initial)
{
    Throttled throttle(100, 50);

    ASSERT_FALSE(throttle(25));
    ASSERT_FALSE(throttle(50));
    ASSERT_FALSE(throttle(75));
    ASSERT_TRUE(throttle(100));

    ASSERT_FALSE(throttle(125));
    ASSERT_FALSE(throttle(25));
    ASSERT_FALSE(throttle(75));
    ASSERT_TRUE(throttle(100));
}

TEST(THROTTLED, short_initial_delay)
{
    Throttled throttle(0, 100);

    ASSERT_TRUE(throttle(25));
    ASSERT_FALSE(throttle(50));
    ASSERT_FALSE(throttle(75));
    ASSERT_FALSE(throttle(100));
    ASSERT_TRUE(throttle(125));
    ASSERT_FALSE(throttle(150));
    ASSERT_FALSE(throttle(175));
    ASSERT_FALSE(throttle(200));
    ASSERT_TRUE(throttle(225));
}

TEST(THROTTLED, short_repeat_delay)
{
    Throttled throttle(100, 0);

    ASSERT_FALSE(throttle(25));
    ASSERT_FALSE(throttle(50));
    ASSERT_FALSE(throttle(75));
    ASSERT_TRUE(throttle(100));
    ASSERT_TRUE(throttle(125));
    ASSERT_TRUE(throttle(150));
}

TEST(THROTTLED, repeated_single_press)
{
    Throttled throttle(25, 100);

    ASSERT_TRUE(throttle(25));
    ASSERT_TRUE(throttle(25));
    ASSERT_FALSE(throttle(50));
    ASSERT_FALSE(throttle(75));
    ASSERT_FALSE(throttle(100));
    ASSERT_TRUE(throttle(125));
    ASSERT_TRUE(throttle(25));
}

TEST(THROTTLED, delay_before_accumulate)
{
    Throttled throttle(25, 100);

    ASSERT_TRUE(throttle(1000));
    ASSERT_FALSE(throttle(1025));
    ASSERT_FALSE(throttle(1050));
    ASSERT_FALSE(throttle(1075));
    ASSERT_TRUE(throttle(1100));
}
