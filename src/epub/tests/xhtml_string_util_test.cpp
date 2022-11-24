#include "../xhtml_string_util.h"

#include <gtest/gtest.h>

TEST(XHTML_STRING_UTIL, compact_whitespace)
{
    EXPECT_EQ(compact_whitespace(""), "");
    EXPECT_EQ(compact_whitespace(" "), " ");
    EXPECT_EQ(compact_whitespace("  "), " ");
    EXPECT_EQ(compact_whitespace("foo  bar"), "foo bar");
    EXPECT_EQ(compact_whitespace("foo\rbar"), "foo bar");
    EXPECT_EQ(compact_whitespace("  foo\r\n\nbar\t"), " foo bar ");
}

TEST(XHTML_STRING_UTIL, remove_carriage_return)
{
    EXPECT_EQ(remove_carriage_returns(""), "");
    EXPECT_EQ(remove_carriage_returns("\rfoo\r\n\tbar "), "foo\n\tbar ");
    EXPECT_EQ(remove_carriage_returns(" foo\tbar\n "), " foo\tbar\n ");
}

TEST(XHTML_STRING_UTIL, strip_whitespace_left)
{
    auto strip_count = [](const char *str) {
        return strip_whitespace_left(str) - str;
    };

    EXPECT_EQ(strip_count(""), 0);
    EXPECT_EQ(strip_count("  "), 2);
    EXPECT_EQ(strip_count(" \n\t\r "), 5);
    EXPECT_EQ(strip_count("  foo bar "), 2);
    EXPECT_EQ(strip_count("\nfoo bar\t"), 1);
}
