#pragma once

#include <vector/bit_vector.hpp>
#include <vector/bit_vector_operations.hpp>
#include <wt/lwt_huffman.hpp>

#include <cstddef>
#include <vector>

// This file contains operations for levelwise wavelet trees external to the class itself
// as not to clutter it

inline bit_vector lwt_huffman_get(const lwt_huffman& t, size_t i) {
    std::vector<bool> bits;

    size_t start = 0;
    size_t end = t.size() - 1;

    // using canonical encoding of huffman code
    for (size_t l = 0; l < t.height() && i < t.size(l); l++) {
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

static inline size_t lwt_huffman_height(const lwt_huffman& t) {
    return t.height();
}

static inline size_t lwt_huffman_size(const lwt_huffman& t, size_t level = 0) {
    return t.size(level);
}
