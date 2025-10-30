#include "../include/cache/metrics.hpp"
#include <vector>

namespace cache {

Metrics::Metrics(const std::string& /*cache_name*/) {
#ifdef HAS_PROMETHEUS
    registry_ = std::make_shared<prometheus::Registry>();
    auto& hits_family = prometheus::BuildCounter()
        .Name("cache_hits_total").Help("Cache hit count").Register(*registry_);
    auto& misses_family = prometheus::BuildCounter()
        .Name("cache_misses_total").Help("Cache miss count").Register(*registry_);
    auto& evictions_family = prometheus::BuildCounter()
        .Name("cache_evictions_total").Help("Cache eviction count").Register(*registry_);
    auto& size_family = prometheus::BuildGauge()
        .Name("cache_size").Help("Cache size").Register(*registry_);
    std::vector<double> buckets{1000, 2000, 5000, 10000, 50000, 100000, 500000, 1000000};
    auto& latency_family = prometheus::BuildHistogram()
        .Name("cache_op_latency_ns").Help("Cache operation latency (ns)")
        .Register(*registry_);
    prometheus_hits_ = &hits_family.Add({});
    prometheus_misses_ = &misses_family.Add({});
    prometheus_evictions_ = &evictions_family.Add({});
    prometheus_size_ = &size_family.Add({});
    prometheus_latency_ = &latency_family.Add({}, buckets);
#endif
}

void Metrics::record_hit() {
    hits_.fetch_add(1, std::memory_order_relaxed);
#ifdef HAS_PROMETHEUS
    if (prometheus_hits_) prometheus_hits_->Increment();
#endif
}

void Metrics::record_miss() {
    misses_.fetch_add(1, std::memory_order_relaxed);
#ifdef HAS_PROMETHEUS
    if (prometheus_misses_) prometheus_misses_->Increment();
#endif
}

void Metrics::record_eviction() {
    evictions_.fetch_add(1, std::memory_order_relaxed);
#ifdef HAS_PROMETHEUS
    if (prometheus_evictions_) prometheus_evictions_->Increment();
#endif
}

void Metrics::record_latency_ns(uint64_t latency_ns) {
#ifdef HAS_PROMETHEUS
    if (prometheus_latency_) prometheus_latency_->Observe(static_cast<double>(latency_ns));
#else
    (void)latency_ns;
#endif
}

void Metrics::set_size(size_t size) {
#ifdef HAS_PROMETHEUS
    if (prometheus_size_) prometheus_size_->Set(static_cast<double>(size));
#else
    (void)size;
#endif
}

} // namespace cache

