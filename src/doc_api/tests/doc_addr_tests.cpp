#include "../doc_addr.h"

#include <gtest/gtest.h>

TEST(DOC_ADDR, encode_address)
{
    DocAddr addr = 0x1200000034;
    ASSERT_EQ(encode_address(addr), "0000001200000034");
}

TEST(DOC_ADDR, decode_address)
{
    ASSERT_EQ(decode_address("0000001200000034"), 0x1200000034);
    ASSERT_EQ(decode_address("asdf"), 0);
    ASSERT_EQ(decode_address(""), 0);
}
