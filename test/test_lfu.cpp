#include "../include/cache/lfu_cache.hpp"
#include <gtest/gtest.h>

TEST(LFUCacheTest, BasicPutGet) {
  cache::LFUCache<int, int> c(2);
  EXPECT_TRUE(c.put(1, 10));
  EXPECT_TRUE(c.put(2, 20));
  auto v1 = c.get(1);
  ASSERT_TRUE(v1.has_value());
  EXPECT_EQ(*v1, 10);
}

TEST(LFUCacheTest, EvictionLeastFrequent) {
  cache::LFUCache<int, int> c(2);
  c.put(1, 10);
  c.put(2, 20);
  (void)c.get(1); // freq(1)=2, freq(2)=1
  c.put(3, 30);   // evict 2
  EXPECT_FALSE(c.get(2).has_value());
  EXPECT_TRUE(c.get(1).has_value());
  EXPECT_TRUE(c.get(3).has_value());
}

