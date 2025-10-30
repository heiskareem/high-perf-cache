#include "../include/cache/lru_cache.hpp"
#include "../include/cache/lfu_cache.hpp"
#include "../include/cache/arc_cache.hpp"
#include <iostream>
#include <random>

int main() {
  std::cout << "Mixed workload example" << std::endl;

  cache::LRUCache<int, int> lru(256);
  cache::LFUCache<int, int> lfu(256);
  cache::ARCCache<int, int> arc(256);

  std::mt19937 rng(123);
  std::uniform_int_distribution<int> key(0, 4095);

  for (int i = 0; i < 50000; ++i) {
    int k = key(rng);
    lru.put(k, i);
    lfu.put(k, i);
    arc.put(k, i);
    if (i % 3 == 0) {
      (void)lru.get(k);
      (void)lfu.get(k);
      (void)arc.get(k);
    }
  }

  std::cout << "LRU size/hit_rate: " << lru.size() << "/" << lru.capacity() << ", " << lru.hit_rate() << "\n";
  std::cout << "LFU size/hit_rate: " << lfu.size() << "/" << lfu.capacity() << ", " << lfu.hit_rate() << "\n";
  std::cout << "ARC size/hit_rate: " << arc.size() << "/" << arc.capacity() << ", " << arc.hit_rate() << "\n";
  return 0;
}

