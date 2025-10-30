#include "../include/cache/arc_cache.hpp"
#include <iostream>

int main() {
  std::cout << "ARC example" << std::endl;
  cache::ARCCache<int, int> c(3);
  c.put(1, 10);
  c.put(2, 20);
  c.put(3, 30);
  (void)c.get(1); // recent/frequent adaption
  c.put(4, 40); // eviction happens based on ARC policy
  int evicted = (!c.get(2).has_value()) + (!c.get(3).has_value());
  std::cout << "evicted among {2,3}: " << evicted << "\n";
  std::cout << "size/capacity: " << c.size() << "/" << c.capacity() << "\n";
  std::cout << "hit_rate: " << c.hit_rate() << "\n";
  return 0;
}

