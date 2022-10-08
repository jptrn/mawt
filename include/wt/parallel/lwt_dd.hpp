#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

#include <wt/parallel/dd_merge.hpp>
#include <wt/parallel/lwt_dd_borders.hpp>
#include <wt/parallel/wt_dd_params.hpp>
#include <wt/lwt.hpp>
#include <wt/wt_params.hpp>

#include <omp.h>

#include <util/debug.hpp>
#include <util/time_measure.hpp>

// Like the DD construction algorithm, but with time measurements inserted
template <typename value_type, typename wt_impl_type, typename wt_dd_impl_type>
inline lwt lwt_make_dd(const std::vector<value_type>& s, size_t max_depth, time_measure& tm = TIME_MEASURE_DUMMY) {
    using bit_copy_type = wt_dd_impl_type::bit_copy_type;

    tm.start("ctor");
    tm.start("init");

    size_t shards = omp_get_max_threads();
    std::vector<size_t> shard_starts;
    std::vector<size_t> shard_sizes;
    shard_starts.reserve(shards);
    shard_sizes.reserve(shards);

    size_t start = 0;
    for (size_t i = 0; i < shards; i++) {
        // We follow the PWM scheme for distributing the LWT among shards
        // One could consider rounding shard sizes up to the nearest word or stuff like that, but I doubt it makes a significant difference
        size_t size = (s.size() / shards) + ((i < (s.size() % shards)) ? 1 : 0);
        shard_starts.push_back(start);
        shard_sizes.push_back(size);
        start += size;
    }

    // Create partial LWT slices. They are not created or merged in place, but instead separate in memory
    std::vector<lwt> lwt_slices;
    lwt_slices.reserve(shards);
    std::vector<std::vector<size_t>> borders(shards);
    // Distribute LWT across slices
    for (size_t i = 0; i < shards; i++) {
        // should be O(K) ~ O(1) because we do not actually zero anything here
        lwt_slices.emplace_back(shard_sizes[i], max_depth);
    }

    tm.end("init");

    // Phase 1: Create LWT Shards and remember borders
    tm.start("split");
    #pragma omp parallel
    {
        size_t k = omp_get_thread_num();
        size_t start = shard_starts[k];
        size_t end = start + shard_sizes[k];
        lwt_slices[k].make_inplace<value_type, wt_impl_type>(s, start, end, &borders[k]);
    }
    tm.end("split");

    // Merge
    tm.start("merge");
    auto merged = wt_merge<lwt, lwt_dd_borders, bit_copy_type>(s.size(), max_depth, lwt_slices, lwt_dd_borders(borders, max_depth));
    tm.end("merge");
    tm.end("ctor");
    return merged;
}