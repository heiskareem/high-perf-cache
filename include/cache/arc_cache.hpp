#pragma once
#include "cache_interface.hpp"
#include "memory_allocator.hpp"
#include "metrics.hpp"
#include <unordered_map>
#include <list>
#include <shared_mutex>
#include <chrono>

namespace cache {

// Adaptive Replacement Cache (ARC)
// Balances between recency (LRU) and frequency (LFU)
template<typename Key, typename Value>
class ARCCache : public CacheInterface<Key, Value> {
private:
    struct Node {
        Key key;
        Value value;
    };
    
    using NodeList = std::list<Node>;
    
    enum class ListType { T1, T2, B1, B2 };
    
    struct MapValue {
        typename NodeList::iterator it;
        ListType list_type;
    };
    
public:
    explicit ARCCache(size_t capacity) 
        : capacity_(capacity)
        , p_(0)
        , t1_list_()
        , t2_list_()
        , b1_list_()
        , b2_list_()
        , metrics_("arc_cache") {}
    
    bool put(const Key& key, const Value& value) override {
        auto start = std::chrono::high_resolution_clock::now();
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto map_it = map_.find(key);
        
        // Case 1: Key in T1 or T2 (cache hit)
        if (map_it != map_.end() && 
            (map_it->second.list_type == ListType::T1 || 
             map_it->second.list_type == ListType::T2)) {
            
            // Move to MRU of T2
            if (map_it->second.list_type == ListType::T1) {
                t1_list_.erase(map_it->second.it);
            } else {
                t2_list_.erase(map_it->second.it);
            }
            
            t2_list_.push_front({key, value});
            map_[key] = {t2_list_.begin(), ListType::T2};
            
            auto end = std::chrono::high_resolution_clock::now();
            metrics_.record_latency_ns(
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            );
            return true;
        }
        
        // Case 2: Key in B1 (ghost from T1)
        if (map_it != map_.end() && map_it->second.list_type == ListType::B1) {
            // Adapt: increase p
            size_t b1_size = b1_list_.size();
            size_t b2_size = b2_list_.size();
            size_t delta = b2_size >= b1_size ? 1 : b2_size / b1_size + 1;
            p_ = std::min(capacity_, p_ + delta);
            
            replace(key, ListType::B1);
            b1_list_.erase(map_it->second.it);
            
            t2_list_.push_front({key, value});
            map_[key] = {t2_list_.begin(), ListType::T2};
            
            auto end = std::chrono::high_resolution_clock::now();
            metrics_.record_latency_ns(
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            );
            return true;
        }
        
        // Case 3: Key in B2 (ghost from T2)
        if (map_it != map_.end() && map_it->second.list_type == ListType::B2) {
            // Adapt: decrease p
            size_t b1_size = b1_list_.size();
            size_t b2_size = b2_list_.size();
            size_t delta = b1_size >= b2_size ? 1 : b1_size / b2_size + 1;
            p_ = (p_ >= delta) ? p_ - delta : 0;
            
            replace(key, ListType::B2);
            b2_list_.erase(map_it->second.it);
            
            t2_list_.push_front({key, value});
            map_[key] = {t2_list_.begin(), ListType::T2};
            
            auto end = std::chrono::high_resolution_clock::now();
            metrics_.record_latency_ns(
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            );
            return true;
        }
        
        // Case 4: New key
        size_t l1_size = t1_list_.size() + b1_list_.size();
        size_t l2_size = t2_list_.size() + b2_list_.size();
        
        if (l1_size == capacity_) {
            if (t1_list_.size() < capacity_) {
                auto& lru = b1_list_.back();
                map_.erase(lru.key);
                b1_list_.pop_back();
                replace(key, ListType::B1);
            } else {
                auto& lru = t1_list_.back();
                map_.erase(lru.key);
                t1_list_.pop_back();
                metrics_.record_eviction();
            }
        } else if (l1_size < capacity_ && l1_size + l2_size >= capacity_) {
            if (l1_size + l2_size >= 2 * capacity_) {
                auto& lru = b2_list_.back();
                map_.erase(lru.key);
                b2_list_.pop_back();
            }
            replace(key, ListType::B1);
        }
        
        t1_list_.push_front({key, value});
        map_[key] = {t1_list_.begin(), ListType::T1};
        
        auto end = std::chrono::high_resolution_clock::now();
        metrics_.record_latency_ns(
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        );
        metrics_.set_size(t1_list_.size() + t2_list_.size());
        
        return true;
    }
    
    std::optional<Value> get(const Key& key) override {
        auto start = std::chrono::high_resolution_clock::now();
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto map_it = map_.find(key);
        if (map_it == map_.end() || 
            map_it->second.list_type == ListType::B1 || 
            map_it->second.list_type == ListType::B2) {
            metrics_.record_miss();
            return std::nullopt;
        }
        
        Value val = map_it->second.it->value;
        
        // Move to MRU of T2
        if (map_it->second.list_type == ListType::T1) {
            Node node = *map_it->second.it;
            t1_list_.erase(map_it->second.it);
            t2_list_.push_front(node);
            map_[key] = {t2_list_.begin(), ListType::T2};
        } else {
            t2_list_.splice(t2_list_.begin(), t2_list_, map_it->second.it);
        }
        
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
        
        switch (map_it->second.list_type) {
            case ListType::T1: t1_list_.erase(map_it->second.it); break;
            case ListType::T2: t2_list_.erase(map_it->second.it); break;
            case ListType::B1: b1_list_.erase(map_it->second.it); break;
            case ListType::B2: b2_list_.erase(map_it->second.it); break;
        }
        
        map_.erase(map_it);
        metrics_.set_size(t1_list_.size() + t2_list_.size());
        
        return true;
    }
    
    void clear() override {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        map_.clear();
        t1_list_.clear();
        t2_list_.clear();
        b1_list_.clear();
        b2_list_.clear();
        p_ = 0;
        metrics_.set_size(0);
    }
    
    size_t size() const override {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return t1_list_.size() + t2_list_.size();
    }
    
    size_t capacity() const override { return capacity_; }
    size_t hit_count() const override { return metrics_.hits(); }
    size_t miss_count() const override { return metrics_.misses(); }
    size_t eviction_count() const override { return metrics_.evictions(); }
    double hit_rate() const override { return metrics_.hit_rate(); }
    
private:
    void replace(const Key& key, ListType ghost_type) {
        if (!t1_list_.empty() && 
            ((t1_list_.size() > p_) || 
             (ghost_type == ListType::B2 && t1_list_.size() == p_))) {
            
            auto& lru = t1_list_.back();
            Key old_key = lru.key;
            t1_list_.pop_back();
            
            b1_list_.push_front({old_key, Value{}});
            map_[old_key] = {b1_list_.begin(), ListType::B1};
            metrics_.record_eviction();
        } else if (!t2_list_.empty()) {
            auto& lru = t2_list_.back();
            Key old_key = lru.key;
            t2_list_.pop_back();
            
            b2_list_.push_front({old_key, Value{}});
            map_[old_key] = {b2_list_.begin(), ListType::B2};
            metrics_.record_eviction();
        }
    }
    
    const size_t capacity_;
    size_t p_; // Target size for T1
    
    NodeList t1_list_; // Recently accessed once
    NodeList t2_list_; // Recently accessed multiple times
    NodeList b1_list_; // Ghost entries evicted from T1
    NodeList b2_list_; // Ghost entries evicted from T2
    
    std::unordered_map<Key, MapValue> map_;
    mutable std::shared_mutex mutex_;
    Metrics metrics_;
};

} // namespace cache
