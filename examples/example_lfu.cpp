#include "../include/cache/lfu_cache.hpp"
#include <iostream>

int main() {
  std::cout << "LFU example" << std::endl;
  cache::LFUCache<int, int> c(3);
  c.put(1, 10);
  c.put(2, 20);
  c.put(3, 30);
  (void)c.get(1); (void)c.get(1); // freq 1 -> 3
  (void)c.get(2);                 // freq 2 -> 2
  // 3 has freq 1
  c.put(4, 40); // evicts least-frequent (key 3)
  std::cout << "has 3? " << (c.get(3).has_value() ? "yes" : "no") << "\n";
  std::cout << "has 1? " << (c.get(1).has_value() ? "yes" : "no") << "\n";
  std::cout << "size/capacity: " << c.size() << "/" << c.capacity() << "\n";
  std::cout << "hit_rate: " << c.hit_rate() << "\n";
  return 0;
}

