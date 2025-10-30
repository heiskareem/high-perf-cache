#pragma once
#include <atomic>
#include <cstddef>
#include <functional>
#include <optional>

namespace cache {

// Minimal placeholder for a lock-free atomic hashmap interface.
// This header is not used by the build targets yet and can be expanded later.
template <typename Key, typename Value>
class AtomicHashMap {
public:
    explicit AtomicHashMap(size_t /*capacity*/ = 1024) {}
    bool insert(const Key& /*key*/, const Value& /*value*/) { return false; }
    std::optional<Value> get(const Key& /*key*/) const { return std::nullopt; }
    bool erase(const Key& /*key*/) { return false; }
    size_t size() const { return 0; }
};

} // namespace cache

