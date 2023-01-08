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

TEST(XHTML_STRING_UTIL, compact_strings)
{
    // empty input
    EXPECT_EQ(compact_strings({}), "");

    // empty output
    EXPECT_EQ(compact_strings({""}), "");
    EXPECT_EQ(compact_strings({" "}), "");

    // single string
    EXPECT_EQ(compact_strings({"hi"}), "hi");
    EXPECT_EQ(compact_strings({" hi "}), "hi");

    // whitespace on different sides
    EXPECT_EQ(compact_strings({"a", "b"}), "ab");
    EXPECT_EQ(compact_strings({"a", "", "b"}), "ab");
    EXPECT_EQ(compact_strings({"a", "\t\t\t", "b"}), "a b");
    EXPECT_EQ(compact_strings({"a", " ", "\t\t\t", "b"}), "a b");
    EXPECT_EQ(compact_strings({"hi", " there"}), "hi there");
    EXPECT_EQ(compact_strings({"hi ", "there"}), "hi there");
    EXPECT_EQ(compact_strings({"hi ", " there"}), "hi there");

    // starting whitespace
    EXPECT_EQ(compact_strings({"", "hi"}), "hi");
    EXPECT_EQ(compact_strings({" ", "hi"}), "hi");
    EXPECT_EQ(compact_strings({" ", " hi"}), "hi");
    EXPECT_EQ(compact_strings({" ", "", "hi"}), "hi");

    // ending whitespace
    EXPECT_EQ(compact_strings({"hi "}), "hi");
    EXPECT_EQ(compact_strings({"hi ", ""}), "hi");
    EXPECT_EQ(compact_strings({"hi ", "", " "}), "hi");

    // also does compaction
    EXPECT_EQ(compact_strings({"\thi\t\t there\t\t"}), "hi there");
}
