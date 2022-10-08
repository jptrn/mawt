#pragma once

#include <vector/bit_vector.hpp>
#include <vector/bit_vector_operations.hpp>
#include <wt/wm.hpp>

#include <cstddef>
#include <vector>

// This file contains operations for WMs external to the class itself
// as not to clutter it

inline bit_vector wm_get(const wm& m, size_t i) {
    std::vector<bool> bits;
    for (size_t l = 0; l < m.height(); l++) {
        const auto& level = m.level(l);
        bits.insert(bits.begin(), level[i]);
        if (!level[i]) {
            i = bit_vector_rank0(level, i) - 1;
        } else {
            i = bit_vector_rank0(level) + bit_vector_rank1(level, i) - 1;
        }
    }
    return bit_vector(bits);
}

static inline size_t wm_height(const wm& m) {
    return m.height();
}

static inline size_t wm_size(const wm& m) {
    return m.size();
}
