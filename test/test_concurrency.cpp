#include "../include/cache/lru_cache.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

TEST(ConcurrencyTest, MultiThreadedAccess) {
  cache::LRUCache<int, int> c(1024);
  const int threads = 8;
  const int ops = 1000;
  std::vector<std::thread> ts;
  for (int t = 0; t < threads; ++t) {
    ts.emplace_back([&c, t, ops]() {
      for (int i = 0; i < ops; ++i) {
        int k = (t * ops + i) % 2048;
        c.put(k, k);
        (void)c.get(k);
      }
    });
  }
  for (auto& th : ts) th.join();
  EXPECT_LE(c.size(), c.capacity());
}

