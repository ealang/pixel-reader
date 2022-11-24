#include "../doc_addr.h"

#include <gtest/gtest.h>

TEST(DOC_ADDR, make_address)
{
    EXPECT_EQ(make_address(0, 0), 0);
    EXPECT_EQ(make_address(0x12, 0x34), 0x1200000034);
}

TEST(DOC_ADDR, parse_address)
{
    EXPECT_EQ(get_chapter_number(0x1200000034), 0x12);
    EXPECT_EQ(get_text_number(0x1200000034), 0x34);
}

TEST(DOC_ADDR, increment_address)
{
    EXPECT_EQ(increment_address(0x1200000034, 1), 0x1200000035);
}
