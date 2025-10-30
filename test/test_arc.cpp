#include "../include/cache/arc_cache.hpp"
#include <gtest/gtest.h>

TEST(ARCCacheTest, BasicPutGet) {
  cache::ARCCache<int, int> c(2);
  EXPECT_TRUE(c.put(1, 10));
  EXPECT_TRUE(c.put(2, 20));
  auto v1 = c.get(1);
  ASSERT_TRUE(v1.has_value());
  EXPECT_EQ(*v1, 10);
}

TEST(ARCCacheTest, EvictionHappens) {
  cache::ARCCache<int, int> c(2);
  c.put(1, 10);
  c.put(2, 20);
  (void)c.get(1);
  EXPECT_TRUE(c.put(3, 30));
  // At least one of the old keys should be evicted
  int evicted = (!c.get(2).has_value()) + (!c.get(1).has_value());
  EXPECT_GE(evicted, 1);
}

