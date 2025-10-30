#pragma once
#include <optional>
#include <string>
#include <memory>

namespace cache {

template<typename Key, typename Value>
class CacheInterface {
public:
    virtual ~CacheInterface() = default;
    
    virtual bool put(const Key& key, const Value& value) = 0;
    virtual std::optional<Value> get(const Key& key) = 0;
    virtual bool remove(const Key& key) = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
    virtual size_t capacity() const = 0;
    
    // Metrics
    virtual size_t hit_count() const = 0;
    virtual size_t miss_count() const = 0;
    virtual size_t eviction_count() const = 0;
    virtual double hit_rate() const = 0;
};

} // namespace cache

