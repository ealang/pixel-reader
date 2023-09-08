#include "util/string_serialization.h"

#include <gtest/gtest.h>

TEST(TRY_DECODE_UINT, valid)
{
    EXPECT_EQ(try_decode_uint("0"), 0);
    EXPECT_EQ(try_decode_uint("113"), 113);
    EXPECT_EQ(try_decode_uint("-1"), -1);
    EXPECT_EQ(try_decode_uint("10foo"), 10);
}

TEST(TRY_DECODE_UINT, invalid)
{
    EXPECT_EQ(try_decode_uint(""), std::nullopt);
    EXPECT_EQ(try_decode_uint("foo"), std::nullopt);
}

TEST(TRY_DECODE_UINT_VECTOR, empty_string)
{
    std::vector<uint32_t> array;
    ASSERT_TRUE(try_decode_uint_vector("", array));
    ASSERT_EQ(array.size(), 0);
}

TEST(TRY_DECODE_UINT_VECTOR, single_num)
{
    std::vector<uint32_t> array;
    ASSERT_TRUE(try_decode_uint_vector("0", array));
    ASSERT_EQ(array, (std::vector<uint32_t>{0}));
}

TEST(TRY_DECODE_UINT_VECTOR, multiple_nums)
{
    std::vector<uint32_t> array;
    ASSERT_TRUE(try_decode_uint_vector("100,200,5000", array));
    ASSERT_EQ(array, (std::vector<uint32_t>{100, 200, 5000}));
}

TEST(TRY_DECODE_UINT_VECTOR, invalid_number)
{
    std::vector<uint32_t> array;
    ASSERT_FALSE(try_decode_uint_vector("100,foo,300", array));
    ASSERT_EQ(array, (std::vector<uint32_t>{100}));
}

TEST(TRY_DECODE_UINT_VECTOR, invalid_array)
{
    std::vector<uint32_t> array;
    ASSERT_FALSE(try_decode_uint_vector("foo", array));
    ASSERT_EQ(array.size(), 0);
}

TEST(ENCODE_UINT_VECTOR, encoding)
{
    ASSERT_EQ(encode_uint_vector(std::vector<uint32_t>{}), "");
    ASSERT_EQ(encode_uint_vector(std::vector<uint32_t>{0}), "0");
    ASSERT_EQ(encode_uint_vector(std::vector<uint32_t>{0, 100, 200}), "0,100,200");
}
