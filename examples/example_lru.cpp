#include "../include/cache/lru_cache.hpp"
#include <iostream>
#include <string>

int main() {
  std::cout << "LRU example" << std::endl;
  cache::LRUCache<int, std::string> c(3);
  c.put(1, "one");
  c.put(2, "two");
  c.put(3, "three");
  (void)c.get(1); // MRU 1
  c.put(4, "four"); // evicts key 2
  std::cout << "has 2? " << (c.get(2).has_value() ? "yes" : "no") << "\n";
  std::cout << "has 1? " << (c.get(1).has_value() ? "yes" : "no") << "\n";
  std::cout << "size/capacity: " << c.size() << "/" << c.capacity() << "\n";
  std::cout << "hit_rate: " << c.hit_rate() << "\n";
  return 0;
}

