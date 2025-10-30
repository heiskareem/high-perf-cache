#include "../include/cache/lru_cache.hpp"
#include <iostream>
#include <thread>
#include <vector>

int main() {
  std::cout << "Concurrency example" << std::endl;
  cache::LRUCache<int, int> c(1024);
  const int threads = 4;
  const int ops = 20000;
  std::vector<std::thread> ts;
  for (int t = 0; t < threads; ++t) {
    ts.emplace_back([&c, t, ops]() {
      for (int i = 0; i < ops; ++i) {
        int k = (t * ops + i) % 4096;
        c.put(k, k);
        (void)c.get(k);
      }
    });
  }
  for (auto& th : ts) th.join();
  std::cout << "final size/capacity: " << c.size() << "/" << c.capacity() << "\n";
  std::cout << "hit_rate: " << c.hit_rate() << "\n";
  return 0;
}

