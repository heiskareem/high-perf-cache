#pragma once
#include <vector>
#include <random>

namespace bench {

inline std::vector<int> sequential_keys(size_t n) {
    std::vector<int> keys(n);
    for (size_t i = 0; i < n; ++i) keys[i] = static_cast<int>(i);
    return keys;
}

inline std::vector<int> zipf_keys(size_t n, int max_key, double skew = 1.2) {
    std::vector<int> out;
    out.reserve(n);
    std::mt19937 rng(42);
    std::uniform_real_distribution<> uni(0.0, 1.0);
    // Simple inverse CDF approximation
    for (size_t i = 0; i < n; ++i) {
        double u = uni(rng);
        int k = static_cast<int>(max_key * std::pow(u, -1.0 / skew)) % max_key;
        if (k < 0) k = -k;
        out.push_back(k);
    }
    return out;
}

} // namespace bench

