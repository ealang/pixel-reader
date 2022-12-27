#include "../lru_cache.h"

#include <gtest/gtest.h>

using lru_cache = LRUCache<std::string, int>;

TEST(LRU_CACHE, add_retrieve)
{
    auto k = "0";
    int v = 1;

    lru_cache cache;
    ASSERT_FALSE(cache.has(k));
    ASSERT_EQ(0, cache.size());
    cache.put(k, v);

    ASSERT_TRUE(cache.has(k));
    ASSERT_EQ(cache[k], v);
    ASSERT_EQ(1, cache.size());
    ASSERT_EQ(cache.back_key(), k);
    ASSERT_EQ(cache.back_value(), v);
}

TEST(LRU_CACHE, orders_by_insert)
{
    lru_cache cache;
    cache.put("0", 0);
    cache.put("1", 1);
    cache.put("2", 2);
    ASSERT_EQ(cache.back_key(), "0");
    cache.pop();
    ASSERT_EQ(cache.back_key(), "1");
    cache.pop();
    ASSERT_EQ(cache.back_key(), "2");
}

TEST(LRU_CACHE, orders_by_retrieval)
{
    lru_cache cache;
    cache.put("0", 0);
    cache.put("1", 1);
    cache.put("2", 2);

    ASSERT_EQ(cache["1"], 1);

    ASSERT_EQ(cache.back_key(), "0");
    cache.pop();
    ASSERT_EQ(cache.back_key(), "2");
    cache.pop();
    ASSERT_EQ(cache.back_key(), "1");
}

TEST(LRU_CACHE, replace)
{
    lru_cache cache;
    cache.put("0", 0);
    cache.put("0", 0);
    ASSERT_EQ(cache.size(), 1);

    cache.put("0", 100);
    ASSERT_EQ(cache.size(), 1);
    ASSERT_EQ(cache["0"], 100);
}
