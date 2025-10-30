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
- `cache_lib` – static library
- `example`, `example_lru`, `example_lfu`, `example_arc`, `example_concurrency`, `example_mixed`
- `examples` – builds all example executables
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

Run all examples (WSL/Linux):

```
./scripts/run_examples.sh
```

On Windows PowerShell:

```
./scripts/run_examples.ps1
```

The runner script auto-detects the build directory in this order: `build`, `build-linux`, `build-gcc`.

## Example Output

Running the examples via `./scripts/run_examples.sh` on Ubuntu/WSL produces output like:

```
Running example
1 => one
hit_rate=1
size=2/2

Running example_lru
LRU example
has 2? no
has 1? yes
size/capacity: 3/3
hit_rate: 0.666667

Running example_lfu
LFU example
has 3? no
has 1? yes
size/capacity: 3/3
hit_rate: 0.8

Running example_arc
ARC example
evicted among {2,3}: 1
size/capacity: 3/3
hit_rate: 0.666667

Running example_concurrency
Concurrency example
final size/capacity: 1024/1024
hit_rate: 1

Running example_mixed
Mixed workload example
LRU size/hit_rate: 256/256, 1
LFU size/hit_rate: 256/256, 1
ARC size/hit_rate: 256/256, 1
```

### What the output shows

- `example`: Minimal LRU usage; a successful `get` after `put` yields `hit_rate=1` and size equals capacity.
- `example_lru`: Demonstrates LRU eviction. Accessing key `1` makes it MRU; inserting key `4` evicts key `2` (LRU). Hit rate reflects 2 hits out of 3 lookups.
- `example_lfu`: Demonstrates LFU eviction. Key `1` is accessed multiple times, so inserting key `4` evicts least-frequent key `3`.
- `example_arc`: Demonstrates ARC adaptation and eviction. One of `{2,3}` is evicted depending on recency/frequency balance.
- `example_concurrency`: Parallel puts/gets across threads; final size is capped at capacity and immediate gets yield a high hit rate.
- `example_mixed`: Runs LRU/LFU/ARC in parallel under a synthetic mixed workload; sizes reach capacity and the scripted access pattern produces a perfect hit rate for demonstration.
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
