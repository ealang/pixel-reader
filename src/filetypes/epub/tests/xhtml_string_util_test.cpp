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
