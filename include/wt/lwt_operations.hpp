#pragma once

#include <vector/bit_vector.hpp>
#include <vector/bit_vector_operations.hpp>
#include <wt/lwt.hpp>

#include <cstddef>
#include <vector>

// This file contains operations for levelwise wavelet trees external to the class itself
// as not to clutter it

inline bit_vector lwt_get(const lwt& t, size_t i) {
    std::vector<bool> bits;

    size_t start = 0;
    size_t end = t.size() - 1;

    for (size_t l = 0; l < t.height(); l++) {
        const auto& level = t.level(l);
        bits.insert(bits.begin(), level[i]);
        auto zero_count = bit_vector_rank0(level, start, end);
        if (!level[i]) {
            i = start + bit_vector_rank0(level, start, i) - 1;
            end = start + zero_count - 1;
        } else {
            i = start + zero_count + bit_vector_rank1(level, start, i) - 1;
            start += zero_count;
        }
    }

    return bit_vector(bits);
}

static inline size_t lwt_height(const lwt& t) {
    return t.height();
}

static inline size_t lwt_size(const lwt& t) {
    return t.size();
}
