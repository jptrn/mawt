#pragma once

#include <cstddef>
#include <cstring>
#include <vector>

#include <wt/parallel/dd_merge_bv.hpp>
#include <wt/parallel/dd_merge_bv_simd.hpp>

#include <util/debug.hpp>

static inline size_t get_write_start(const std::vector<std::vector<std::vector<size_t>>>& starts, size_t shard, size_t level, size_t interval) {
    return starts[shard][level][interval];
}

static inline size_t get_write_end(const std::vector<std::vector<std::vector<size_t>>>& starts, size_t shard, size_t level, size_t interval, size_t size) {
    // In a level, the starting positions are arranged like this:
    // // Interval 0          | Interval 1         ...
    // [ k = 0 ][ k = 1 ] ... | [ k = 0 ][ k = 1 ] ...
    if (shard < starts.size() - 1) {
        return starts[shard + 1][level][interval];    
    }
    if (interval < starts[shard][level].size() - 1) {
        return starts[0][level][interval + 1];  // might be bad for OMP, but no synchronization should be required
    }
    return size;
}

// Merging WTs in domain decompositon
// `borders_type` is a template parameter of a class that controls access to the intervals of the wavelet tree
// In the case of the LWT, we just remember the borders, so we have the borders of the final level available
// In the case of the WM, we need to compute the borders level-by-level, so we abstract both into a template type
template <typename wt_type, typename borders_type, typename bit_copy_type>
static inline wt_type wt_merge(size_t size, size_t max_depth, const std::vector<wt_type>& slices, const borders_type& borders) {
    wt_type t(size, max_depth);
    size_t shards = slices.size();

    // Zero bits now - this is redundant, as we could zero them while writing 
    // However, this means I don't have to worry about masking bits in the merge phase and can just "add" bits
    // to the grid - making the code slightly simpler
    for (size_t j = 0; j < max_depth; j++) {
        std::memset(t.level(j).data().data(), 0, t.level(j).data().size() * 8);
    }
    
    // For each shard and each level, remap the intervals for writing
    std::vector<std::vector<std::vector<size_t>>> write_i_starts(shards);
    for (size_t k = 0; k < shards; k++) {
        write_i_starts[k].resize(max_depth);
    }
    for (size_t j = 0; j < max_depth; j++) {
        size_t current = 0;
        for (size_t m = 0; m < (1ULL << j); m++) {
            for (size_t k = 0; k < shards; k++) {
                write_i_starts[k][j].push_back(current);
                size_t prev_border = borders.get_interval_start(k, j, m);
                current += borders.get_interval_end(k, j, m) - prev_border;
            }
        }
    }

    // Copy words
    #pragma omp parallel for
    for (size_t k = 0; k < shards; k++) {   
        for (size_t j = 0; j < max_depth; j++) {
            for (size_t m = 0; m < (1ULL << j); m++) {
                size_t write_start = get_write_start(write_i_starts, k, j, m);
                size_t write_end = get_write_end(write_i_starts, k, j, m, t.size());
                size_t read_i = borders.get_interval_start(k, j, m);
                copy_interval_words<bit_copy_type>(slices[k].level(j), t.level(j), write_start, write_end, read_i);
            }
        }
    }

    // Now deal with edge boundaries, this part is necessarily sequential (we could parallelize over the levels though)
    for (size_t k = 0; k < shards; k++) {   
        for (size_t j = 0; j < max_depth; j++) {
            for (size_t m = 0; m < (1ULL << j); m++) {
                size_t write_start = get_write_start(write_i_starts, k, j, m);
                size_t write_end = get_write_end(write_i_starts, k, j, m, t.size());
                size_t read_i = borders.get_interval_start(k, j, m);
                copy_interval_boundaries<bit_copy_type>(slices[k].level(j), t.level(j), write_start, write_end, read_i);
            }
        }
    }

    return t;
}