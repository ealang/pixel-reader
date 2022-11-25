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

TEST(DOC_ADDR, encode_address)
{
    DocAddr addr = 0x1200000034;
    ASSERT_EQ(encode_address(addr), "v1 0000001200000034");
}

TEST(DOC_ADDR, decode_address)
{
    ASSERT_EQ(decode_address("v1 0000001200000034"), 0x1200000034);
    ASSERT_EQ(decode_address("v2 0000001200000034"), 0);
    ASSERT_EQ(decode_address(""), 0);
}
