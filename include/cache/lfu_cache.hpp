#pragma once
#include "cache_interface.hpp"
#include "memory_allocator.hpp"
#include "metrics.hpp"
#include <unordered_map>
#include <map>
#include <list>
#include <shared_mutex>
#include <chrono>

namespace cache {

template<typename Key, typename Value>
class LFUCache : public CacheInterface<Key, Value> {
private:
    struct Node {
        Key key;
        Value value;
        size_t freq;
    };
    
    using NodeList = std::list<Node, PoolAllocator<Node>>;
    
public:
    explicit LFUCache(size_t capacity) 
        : capacity_(capacity)
        , min_freq_(0)
        , metrics_("lfu_cache") {}
    
    bool put(const Key& key, const Value& value) override {
        auto start = std::chrono::high_resolution_clock::now();
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        if (capacity_ == 0) return false;
        
        auto map_it = map_.find(key);
        if (map_it != map_.end()) {
            // Update existing
            map_it->second.it->value = value;
            touch(key);
        } else {
            // Insert new
            if (map_.size() >= capacity_) {
                evict();
            }
            
            freq_map_[1].push_front({key, value, 1});
            map_[key] = {freq_map_[1].begin(), 1};
            min_freq_ = 1;
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
        
        Value val = map_it->second.it->value;
        touch(key);
        metrics_.record_hit();
        
        auto end = std::chrono::high_resolution_clock::now();
        metrics_.record_latency_ns(
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        );
        
        return val;
    }
    
    bool remove(const Key& key) override {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto map_it = map_.find(key);
        if (map_it == map_.end()) {
            return false;
        }
        
        size_t freq = map_it->second.freq;
        freq_map_[freq].erase(map_it->second.it);
        if (freq_map_[freq].empty()) {
            freq_map_.erase(freq);
        }
        map_.erase(map_it);
        metrics_.set_size(map_.size());
        
        return true;
    }
    
    void clear() override {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        map_.clear();
        freq_map_.clear();
        min_freq_ = 0;
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
    void touch(const Key& key) {
        auto& item = map_[key];
        size_t freq = item.freq;
        Node node = *item.it;
        
        freq_map_[freq].erase(item.it);
        if (freq_map_[freq].empty()) {
            freq_map_.erase(freq);
            if (min_freq_ == freq) {
                min_freq_++;
            }
        }
        
        node.freq++;
        freq_map_[node.freq].push_front(node);
        map_[key] = {freq_map_[node.freq].begin(), node.freq};
    }
    
    void evict() {
        if (freq_map_.empty()) return;
        
        auto& min_list = freq_map_[min_freq_];
        auto& back = min_list.back();
        map_.erase(back.key);
        min_list.pop_back();
        
        if (min_list.empty()) {
            freq_map_.erase(min_freq_);
        }
        
        metrics_.record_eviction();
    }
    
    struct MapValue {
        typename NodeList::iterator it;
        size_t freq;
    };
    
    const size_t capacity_;
    size_t min_freq_;
    std::unordered_map<Key, MapValue> map_;
    std::map<size_t, NodeList> freq_map_;
    mutable std::shared_mutex mutex_;
    Metrics metrics_;
};

} // namespace cache

