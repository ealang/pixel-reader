#include "../utf8.h"

#include <gtest/gtest.h>

static uint32_t step_amount(const char *str)
{
    const char *next = utf8_step(str);
    return next - str;
}

TEST(UTF8, utf8_step_handles_ascii)
{
    EXPECT_EQ(step_amount("asdf"), 1);
}

TEST(UTF8, utf8_step_handles_non_ascii)
{
    EXPECT_GT(step_amount("λ"), 1);
}

TEST(UTF8, is_valid_utf8)
{
    EXPECT_TRUE(is_valid_utf8("", 0));
    EXPECT_TRUE(is_valid_utf8("asdf", 1));
    EXPECT_TRUE(is_valid_utf8("asdf", strlen("asdf")));
    EXPECT_TRUE(is_valid_utf8("λ", strlen("λ")));

    std::vector<uint8_t> utf_16 {
        // hello world
        0xff, 0xfe, 0x68, 0x0, 0x65, 0x0, 0x6c, 0x0, 0x6c, 0x0, 0x6f, 0x0, 0x20, 0x0, 0x77, 0x0, 0x6f, 0x0, 0x72, 0x0, 0x6c, 0x0, 0x64, 0x0, 0xa, 0x0
    };
    EXPECT_FALSE(is_valid_utf8((const char *)utf_16.data(), utf_16.size()));
}
