#include "../str_utils.h"

#include <gtest/gtest.h>

TEST(STR_UTILS, to_lower)
{
    EXPECT_EQ(to_lower(nullptr), "");
    EXPECT_EQ(to_lower(".ePUB"), ".epub");
}

TEST(STR_UTILS, remove_carriage_return)
{
    EXPECT_EQ(remove_carriage_returns(nullptr), "");
    EXPECT_EQ(remove_carriage_returns(""), "");
    EXPECT_EQ(remove_carriage_returns("\rfoo\r\n\tbar "), "foo\n\tbar ");
    EXPECT_EQ(remove_carriage_returns(" foo\tbar\n "), " foo\tbar\n ");
}

TEST(STR_UTILS, strip_whitespace)
{
    EXPECT_EQ(strip_whitespace(nullptr), "");
    EXPECT_EQ(strip_whitespace(""), "");
    EXPECT_EQ(strip_whitespace(" "), "");
    EXPECT_EQ(strip_whitespace("asdf"), "asdf");
    EXPECT_EQ(strip_whitespace(" \tasdf\t\n\r "), "asdf");
}

TEST(STR_UTILS, strip_whitespace_left)
{
    auto strip_count = [](const char *str) {
        return strip_whitespace_left(str) - str;
    };

    EXPECT_EQ(strip_whitespace_left(nullptr), nullptr);
    EXPECT_EQ(strip_count(""), 0);
    EXPECT_EQ(strip_count("  "), 2);
    EXPECT_EQ(strip_count(" \n\t\r "), 5);
    EXPECT_EQ(strip_count("  foo bar "), 2);
    EXPECT_EQ(strip_count("\nfoo bar\t"), 1);
}
