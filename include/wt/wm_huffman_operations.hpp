#pragma once

#include <vector/bit_vector.hpp>
#include <vector/bit_vector_operations.hpp>
#include <wt/wm_huffman.hpp>

#include <cstddef>
#include <vector>

inline bit_vector wm_huffman_get(const wm_huffman& m, size_t i) {
    std::vector<bool> bits;
    for (size_t l = 0; l < m.height() && i < m.size(l); l++) {
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

static inline size_t wm_huffman_height(const wm_huffman& m) {
    return m.height();
}

static inline size_t wm_huffman_size(const wm_huffman& m, size_t l) {
    return m.size(l);
}
