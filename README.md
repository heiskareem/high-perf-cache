# High-Performance In-Memory Cache System (C++17)

A production-grade, thread-safe caching library with multiple eviction policies (LRU, LFU, ARC), a custom pool allocator, optional Prometheus metrics, benchmarks, tests, and examples.

## Features

- Eviction policies: LRU, LFU, ARC
- Thread-safe operations with shared_mutex
- Custom pool allocator to reduce fragmentation
- Optional Prometheus metrics integration
- Benchmarks via Google Benchmark
- Tests via GoogleTest
- C++17, CMake build

## Project Structure

```
high-perf-cache/
├── CMakeLists.txt
├── include/
│   ├── cache/
│   │   ├── cache_interface.hpp
│   │   ├── lru_cache.hpp
│   │   ├── lfu_cache.hpp
│   │   ├── arc_cache.hpp
│   │   ├── memory_allocator.hpp
│   │   └── metrics.hpp
│   └── lockfree/
│       └── atomic_map.hpp
├── src/
│   ├── lru_cache.cpp
│   ├── lfu_cache.cpp
│   ├── arc_cache.cpp
│   ├── memory_allocator.cpp
│   └── metrics.cpp
├── benchmark/
│   ├── cache_benchmark.cpp
│   └── workload_patterns.hpp
├── test/
│   ├── test_lru.cpp
│   ├── test_lfu.cpp
│   ├── test_arc.cpp
│   └── test_concurrency.cpp
├── examples/
│   └── example_usage.cpp
└── README.md
```

## Build

Prereqs: CMake (>= 3.15), C++17 compiler, pthreads. Optional: GoogleTest, Google Benchmark, Prometheus C++ client.

Using CMake (out-of-source):

```
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
```

Targets:
- `example` – simple usage
- `cache_lib` – static library
- `cache_tests` – unit tests (if GTest found)
- `cache_benchmark` – benchmarks (if Google Benchmark found)

Dependencies (optional):
- GoogleTest: via package manager or source; CMake enables tests if `GTest::gtest` is found.
- Google Benchmark: install and expose `benchmark::benchmark` to CMake.
- Prometheus C++: provides `prometheus-cpp::core`; if found, `HAS_PROMETHEUS` is defined and metrics export is enabled.

## Example

```cpp
#include "include/cache/lru_cache.hpp"
#include <iostream>

int main() {
  cache::LRUCache<int, std::string> c(2);
  c.put(1, "one");
  c.put(2, "two");
  auto v = c.get(1); // hit moves 1 to MRU
  c.put(3, "three"); // evicts key 2 (LRU)
  std::cout << "hit_rate=" << c.hit_rate() << "\n";
  return 0;
}
```

Build the example:

```
cmake --build build --target example --config Release
./build/example
```

## Metrics

- Built-in counters: hits, misses, evictions, hit rate, operation latency, size.
- If Prometheus is available, the library links to `prometheus-cpp::core` and exposes counters/histograms internally.

## Benchmarks

If Google Benchmark is available, build and run:

```
cmake --build build --target cache_benchmark --config Release
./build/cache_benchmark
```

## Tests

If GoogleTest is available:

```
cmake --build build --target cache_tests --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Notes

- ARC and LFU use lists and maps with a pool allocator for performance.
- Thread-safety via `std::shared_mutex` for reads/writes.
- Tune capacity, allocator pool sizes, and hashers for your workload.
