#pragma once

#include <cstddef>
#include <vector>

class lwt_dd_borders {
private:
    const std::vector<std::vector<size_t>>& m_borders;
    const size_t m_depth;

public:
    inline lwt_dd_borders(const std::vector<std::vector<size_t>>& borders, size_t depth) 
        : m_borders(borders), m_depth(depth) {}

    inline size_t get_interval_start(size_t shard, size_t level, size_t interval) const {
        return interval == 0 ? 0 : get_interval_end(shard, level, interval - 1);
    }

    inline size_t get_interval_end(size_t shard, size_t level, size_t interval) const {
        size_t interval_distance = 1ULL << (m_depth - 1 - level);
        return m_borders[shard][((interval + 1) * interval_distance) - 1];
    }
};