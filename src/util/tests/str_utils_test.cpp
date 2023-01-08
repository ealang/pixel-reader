#include "../str_utils.h"

#include <gtest/gtest.h>

TEST(STR_UTILS, to_lower)
{
    EXPECT_EQ(to_lower(".ePUB"), ".epub");
}

TEST(STR_UTILS, remove_carriage_return)
{
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
    EXPECT_EQ(strip_whitespace_left(""), "");
    EXPECT_EQ(strip_whitespace_left("  "), "");
    EXPECT_EQ(strip_whitespace_left("foo"), "foo");
    EXPECT_EQ(strip_whitespace_left(" \n\t\r "), "");
    EXPECT_EQ(strip_whitespace_left("  foo bar "), "foo bar ");
    EXPECT_EQ(strip_whitespace_left("\nfoo bar\t"), "foo bar\t");
}

TEST(STR_UTILS, strip_whitespace_right)
{
    EXPECT_EQ(strip_whitespace_right(""), "");
    EXPECT_EQ(strip_whitespace_right("  "), "");
    EXPECT_EQ(strip_whitespace_right("foo"), "foo");
    EXPECT_EQ(strip_whitespace_right(" \n\t\r "), "");
    EXPECT_EQ(strip_whitespace_right("  foo bar "), "  foo bar");
    EXPECT_EQ(strip_whitespace_right("\nfoo bar\t"), "\nfoo bar");
}

TEST(STR_UTILS, convert_tabs_to_space)
{
    EXPECT_EQ(convert_tabs_to_space("", 4), "");
    EXPECT_EQ(convert_tabs_to_space("foo", 4), "foo");
    EXPECT_EQ(convert_tabs_to_space("\tfoo\t", 2), "  foo  ");
    EXPECT_EQ(convert_tabs_to_space("\tf\too", 0), "foo");
}

TEST(STR_UTILS, join_strings)
{
    EXPECT_EQ(join_strings({}), "");
    EXPECT_EQ(join_strings({""}), "");
    EXPECT_EQ(join_strings({"a"}), "a");
    EXPECT_EQ(join_strings({"a", "b"}), "ab");
    EXPECT_EQ(join_strings({"a", "", "b", ""}), "ab");
}
