#include "../utf8.h"

#include <gtest/gtest.h>

static uint32_t step_amount(const char *str)
{
    const char *next = utf8_step(str);
    return next - str;
}

TEST(UTF8, handles_ascii)
{
    EXPECT_EQ(step_amount("asdf"), 1);
}

TEST(UTF8, handles_non_ascii)
{
    EXPECT_GT(step_amount("λ"), 1);
}
