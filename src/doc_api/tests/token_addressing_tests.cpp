#include "../token_addressing.h"

#include <gtest/gtest.h>

TEST(TOKEN_ADDRESSING, get_address_width)
{
    EXPECT_EQ(get_address_width(""), 0);
    EXPECT_EQ(get_address_width(" "), 0);
    EXPECT_EQ(get_address_width("asdf"), 4);
    EXPECT_EQ(get_address_width("\tasdf λv\n\r"), 6);
}
