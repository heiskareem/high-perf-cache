#include "../include/cache/lru_cache.hpp"
#include <iostream>
#include <string>

int main() {
  cache::LRUCache<int, std::string> c(2);
  c.put(1, "one");
  c.put(2, "two");
  if (auto v = c.get(1)) {
    std::cout << "1 => " << *v << "\n";
  }
  c.put(3, "three"); // evicts key 2
  std::cout << "hit_rate=" << c.hit_rate() << "\n";
  std::cout << "size=" << c.size() << "/" << c.capacity() << "\n";
  return 0;
}

