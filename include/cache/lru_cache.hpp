#pragma once
#include "cache_interface.hpp"
#include "memory_allocator.hpp"
#include "metrics.hpp"
#include <unordered_map>
#include <list>
#include <shared_mutex>
#include <chrono>

namespace cache {

template<typename Key, typename Value>
class LRUCache : public CacheInterface<Key, Value> {
private:
    struct Node {
        Key key;
        Value value;
        typename std::list<Node>::iterator list_it;
    };
    
    using NodeList = std::list<Node>;
    using MapType = std::unordered_map<Key, typename NodeList::iterator>;
    
public:
    explicit LRUCache(size_t capacity) 
        : capacity_(capacity)
        , node_list_()
        , metrics_("lru_cache") {}
    
    bool put(const Key& key, const Value& value) override {
        auto start = std::chrono::high_resolution_clock::now();
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto map_it = map_.find(key);
        if (map_it != map_.end()) {
            // Update existing
            node_list_.splice(node_list_.begin(), node_list_, map_it->second);
            map_it->second->value = value;
        } else {
            // Insert new
            if (map_.size() >= capacity_) {
                evict();
            }
            node_list_.push_front({key, value, {}});
            node_list_.front().list_it = node_list_.begin();
            map_[key] = node_list_.begin();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        metrics_.record_latency_ns(
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        );
        metrics_.set_size(map_.size());
        
        return true;
    }
    
    std::optional<Value> get(const Key& key) override {
        auto start = std::chrono::high_resolution_clock::now();
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto map_it = map_.find(key);
        if (map_it == map_.end()) {
            metrics_.record_miss();
            return std::nullopt;
        }
        
        // Move to front (most recently used)
        node_list_.splice(node_list_.begin(), node_list_, map_it->second);
        metrics_.record_hit();
        
        auto end = std::chrono::high_resolution_clock::now();
        metrics_.record_latency_ns(
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        );
        
        return map_it->second->value;
    }
    
    bool remove(const Key& key) override {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto map_it = map_.find(key);
        if (map_it == map_.end()) {
            return false;
        }
        
        node_list_.erase(map_it->second);
        map_.erase(map_it);
        metrics_.set_size(map_.size());
        
        return true;
    }
    
    void clear() override {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        map_.clear();
        node_list_.clear();
        metrics_.set_size(0);
    }
    
    size_t size() const override {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return map_.size();
    }
    
    size_t capacity() const override { return capacity_; }
    size_t hit_count() const override { return metrics_.hits(); }
    size_t miss_count() const override { return metrics_.misses(); }
    size_t eviction_count() const override { return metrics_.evictions(); }
    double hit_rate() const override { return metrics_.hit_rate(); }
    
private:
    void evict() {
        if (node_list_.empty()) return;
        
        auto& back = node_list_.back();
        map_.erase(back.key);
        node_list_.pop_back();
        metrics_.record_eviction();
    }
    
    const size_t capacity_;
    NodeList node_list_;
    MapType map_;
    mutable std::shared_mutex mutex_;
    Metrics metrics_;
};

} // namespace cache
