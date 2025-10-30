#include "../include/cache/lru_cache.hpp"
#include <gtest/gtest.h>

TEST(LRUCacheTest, BasicPutGet) {
  cache::LRUCache<int, int> c(2);
  EXPECT_TRUE(c.put(1, 10));
  EXPECT_TRUE(c.put(2, 20));
  auto v1 = c.get(1);
  ASSERT_TRUE(v1.has_value());
  EXPECT_EQ(*v1, 10);
}

TEST(LRUCacheTest, Eviction) {
  cache::LRUCache<int, int> c(2);
  c.put(1, 10);
  c.put(2, 20);
  c.get(1);           // 1 is MRU, 2 is LRU
  c.put(3, 30);       // evict 2
  EXPECT_FALSE(c.get(2).has_value());
  EXPECT_TRUE(c.get(1).has_value());
  EXPECT_TRUE(c.get(3).has_value());
}

