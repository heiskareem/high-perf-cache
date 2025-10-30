#pragma once
#include <atomic>
#include <memory>
#include <string>
#include <cstdint>

#ifdef HAS_PROMETHEUS
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>
#endif

namespace cache {

class Metrics {
public:
    Metrics(const std::string& cache_name = "default");
    ~Metrics() = default;
    
    void record_hit();
    void record_miss();
    void record_eviction();
    void record_latency_ns(uint64_t latency_ns);
    void set_size(size_t size);
    
    uint64_t hits() const { return hits_.load(std::memory_order_relaxed); }
    uint64_t misses() const { return misses_.load(std::memory_order_relaxed); }
    uint64_t evictions() const { return evictions_.load(std::memory_order_relaxed); }
    
    double hit_rate() const {
        uint64_t h = hits();
        uint64_t m = misses();
        return (h + m) > 0 ? static_cast<double>(h) / (h + m) : 0.0;
    }
    
private:
    std::atomic<uint64_t> hits_{0};
    std::atomic<uint64_t> misses_{0};
    std::atomic<uint64_t> evictions_{0};
    
#ifdef HAS_PROMETHEUS
    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::Counter* prometheus_hits_;
    prometheus::Counter* prometheus_misses_;
    prometheus::Counter* prometheus_evictions_;
    prometheus::Histogram* prometheus_latency_;
    prometheus::Gauge* prometheus_size_;
#endif
};

} // namespace cache

