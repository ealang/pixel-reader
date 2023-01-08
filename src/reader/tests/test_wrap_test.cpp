#include "reader/text_wrap.h"

#include <gtest/gtest.h>

static bool fits_on_line_by_char(const char *, int strlen)
{
    return strlen <= 12;
}

static std::vector<std::string> default_invocation(const char *str, unsigned int max_search=100)
{
    std::vector<std::string> lines;
    wrap_lines(
        str,
        fits_on_line_by_char,
        [&lines](const char *str, uint32_t len) {
            lines.emplace_back(str, len);
        },
        max_search
    );
    return lines;
}

TEST(TEXT_WRAP, empty_string)
{
    EXPECT_EQ(
        default_invocation(""),
        (std::vector<std::string>{""})
    );
}


TEST(TEXT_WRAP, pass_whitespace)
{
    EXPECT_EQ(
        default_invocation("\t  "),
        (std::vector<std::string>{"\t  "})
    );
}

TEST(TEXT_WRAP, short_line)
{
    EXPECT_EQ(
        default_invocation("1234"),
        (std::vector<std::string>{"1234"})
    );

    EXPECT_EQ(
        default_invocation("1234 67"),
        (std::vector<std::string>{"1234 67"})
    );
}

TEST(TEXT_WRAP, exact_fit_line)
{
    EXPECT_EQ(
        default_invocation("123456789abc"),
        (std::vector<std::string>{"123456789abc"})
    );

    EXPECT_EQ(
        default_invocation("123456 89abc"),
        (std::vector<std::string>{"123456 89abc"})
    );
}

TEST(TEXT_WRAP, wrap_words)
{
    EXPECT_EQ(
        default_invocation("123456 89 12345"),
        (std::vector<std::string>{"123456 89", "12345"})
    );
    EXPECT_EQ(
        default_invocation("1 3 5 7 9 B D F H"),
        (std::vector<std::string>{"1 3 5 7 9 B", "D F H"})
    );
}

TEST(TEXT_WRAP, emit_literal_line_breaks)
{
    EXPECT_EQ(
        default_invocation("\n"),
        (std::vector<std::string>{""})
    );
    EXPECT_EQ(
        default_invocation("\n\n\n"),
        (std::vector<std::string>{"", "", ""})
    );
    EXPECT_EQ(
        default_invocation("123\n"),
        (std::vector<std::string>{"123"})
    );
    EXPECT_EQ(
        default_invocation("123\n\n"),
        (std::vector<std::string>{"123", ""})
    );
    EXPECT_EQ(
        default_invocation("123\n456"),
        (std::vector<std::string>{"123", "456"})
    );
    EXPECT_EQ(
        default_invocation("123\n\n456"),
        (std::vector<std::string>{"123", "", "456"})
    );
}

TEST(TEXT_WRAP, no_whitespace)
{
    EXPECT_EQ(
        default_invocation("123456789ABCDEF"),
        (std::vector<std::string>{"123456789ABC", "DEF"})
    );

    EXPECT_EQ(
        default_invocation("ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ"),
        (std::vector<std::string>{"ΑΒΓΔΕΖ", "ΗΘΙΚΛΜ", "ΝΞΟΠΡΣ", "ΤΥΦΧΨΩ"})
    );
}

TEST(TEXT_WRAP, support_non_ascii)
{
    EXPECT_EQ(
        default_invocation("ΑΒΓ ΔΕΖ"),
        (std::vector<std::string>{"ΑΒΓ", "ΔΕΖ"})
    );
}

TEST(TEXT_WRAP, limit_mac_char_len)
{
    EXPECT_EQ(
        default_invocation("12345 789ABCD", 3),
        (std::vector<std::string>{"123", "45", "789", "ABC", "D"})
    );
}
