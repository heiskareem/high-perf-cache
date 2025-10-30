#pragma once
#include <memory>
#include <atomic>
#include <cstddef>
#include <vector>
#include <mutex>

namespace cache {

// Memory pool allocator for reduced fragmentation
template<typename T>
class PoolAllocator {
public:
    using value_type = T;
    
    explicit PoolAllocator(size_t pool_size = 10000) 
        : pool_size_(pool_size) {
        allocate_pool();
    }
    
    template<typename U>
    PoolAllocator(const PoolAllocator<U>& other) noexcept 
        : pool_size_(other.pool_size_) {}
    
    T* allocate(size_t n) {
        if (n != 1) {
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        if (free_list_.empty()) {
            allocate_pool();
        }
        
        T* ptr = free_list_.back();
        free_list_.pop_back();
        return ptr;
    }
    
    void deallocate(T* p, size_t n) noexcept {
        if (n != 1) {
            ::operator delete(p);
            return;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        free_list_.push_back(p);
    }
    
    template<typename U>
    struct rebind {
        using other = PoolAllocator<U>;
    };
    
private:
    void allocate_pool() {
        T* pool = static_cast<T*>(::operator new(pool_size_ * sizeof(T)));
        pools_.push_back(pool);
        for (size_t i = 0; i < pool_size_; ++i) {
            free_list_.push_back(pool + i);
        }
    }
    
    size_t pool_size_;
    std::vector<T*> free_list_;
    std::vector<T*> pools_;
    std::mutex mutex_;
    
    template<typename U> friend class PoolAllocator;
};

} // namespace cache

