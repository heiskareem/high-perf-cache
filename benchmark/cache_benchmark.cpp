#include "../include/cache/lru_cache.hpp"
#include "workload_patterns.hpp"
#include <benchmark/benchmark.h>
#include <vector>

static void BM_LRU_PutGet(benchmark::State& state) {
  const int capacity = static_cast<int>(state.range(0));
  cache::LRUCache<int, int> c(capacity);
  auto keys = bench::sequential_keys(100000);
  size_t idx = 0;
  for (auto _ : state) {
    int k = keys[idx++ % keys.size()];
    c.put(k, k);
    (void)c.get(k);
  }
  state.counters["hit_rate"] = c.hit_rate();
}

BENCHMARK(BM_LRU_PutGet)->Arg(1024)->Arg(8192)->Arg(65536);

BENCHMARK_MAIN();

