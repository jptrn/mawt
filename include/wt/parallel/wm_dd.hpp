#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

#include <wt/parallel/dd_merge.hpp>
#include <wt/parallel/wm_dd_borders.hpp>
#include <wt/parallel/wt_dd_params.hpp>
#include <wt/wm.hpp>
#include <wt/wt_params.hpp>

#include <omp.h>

#include <util/debug.hpp>
#include <util/time_measure.hpp>

template <typename value_type, typename wt_impl_type, typename wt_dd_impl_type>
inline wm wm_make_dd(const std::vector<value_type>& s, size_t max_depth, time_measure& tm = TIME_MEASURE_DUMMY) {
    using bit_copy_type = wt_dd_impl_type::bit_copy_type;

    // Like the regular WM algorithm, but has time measures inserted
    // I would really like a way to unify both algorithms

    tm.start("ctor");
    tm.start("init");
    size_t shards = omp_get_max_threads();
    std::vector<size_t> shard_starts;
    std::vector<size_t> shard_sizes;
    shard_starts.reserve(shards);
    shard_sizes.reserve(shards);

    size_t start = 0;
    for (size_t i = 0; i < shards; i++) {
        size_t size = (s.size() / shards) + ((i < (s.size() % shards)) ? 1 : 0);
        shard_starts.push_back(start);
        shard_sizes.push_back(size);
        start += size;
    }

    // Create partial WN slices. They are not created or merged in place, but instead separate in memory
    std::vector<wm> wm_slices;
    wm_slices.reserve(shards);
    // Distribute wm across slices
    for (size_t i = 0; i < shards; i++) {
        wm_slices.emplace_back(shard_sizes[i], max_depth);
    }
    
    tm.end("init");
    tm.start("split");

    // Phase 1: Create wm Shards and remember borders
    #pragma omp parallel
    {
        size_t k = omp_get_thread_num();
        size_t start = shard_starts[k];
        size_t end = start + shard_sizes[k];
        wm_slices[k].make_inplace<value_type, wt_impl_type>(s, start, end);
    }

    tm.end("split");
    tm.start("get_borders");

    // --
    // The WM merge algorithm works the exact same way as the LWT merge algorithm
    // But we need to mind the reversed bit prefixes.
    // Since we do not have those borders available in the WM, we must do another
    // parallel pass to gather them

    // [                   ] [                   ] [                   ]
    // [   0      |  1     ] [   0  |  1         ] [   0   |  1        ]
    // [ 00 | 10 | 01 | 10 ] [ 00 | 10 | 01 | 10 ] [ 00 | 10 | 01 | 10 ]

    // Recreate the interval tree in an intermittent parallel pass
    // v-- k shards  v-- j levels  v-- m intervals
    std::vector<std::vector<std::vector<size_t>>> borders(shards);
    #pragma omp parallel for
    for (size_t k = 0; k < shards; k++) {
        auto& cur_shard = wm_slices[k];
        borders[k].resize(max_depth);
        borders[k][0].push_back(cur_shard.size());
        for (size_t j = 0; j < max_depth - 1; j++) {
            std::vector<size_t> zero_counts;
            std::vector<size_t> one_counts;

            const auto& cur_borders = borders[k][j];
            auto& next_borders = borders[k][j + 1];
            next_borders.resize(cur_borders.size() * 2);

            size_t start = 0;
            for (size_t m = 0; m < (1ULL << j); m++) {
                size_t end = cur_borders[m];
                size_t one_count = count_bits<bit_copy_type>(cur_shard.level(j), start, end);
                size_t zero_count = end - start - one_count;

                next_borders[m] = zero_count;
                next_borders[cur_borders.size() + m] = one_count;

                start = end;
            }

            for (size_t m = 1; m < (1ULL << (j + 1)); m++) {
                next_borders[m] += next_borders[m - 1];
            }
        }
    }

    // Finally, merge the slices using our new borders
    tm.end("get_borders");

    tm.start("merge");
    auto merged = wt_merge<wm, wm_dd_borders, bit_copy_type>(s.size(), max_depth, wm_slices, wm_dd_borders(borders));
    tm.end("merge");
    tm.end("ctor");
    return merged;
}