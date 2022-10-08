#pragma once

#include <cstddef>
#include <vector>

class wm_dd_borders {
private:
    const std::vector<std::vector<std::vector<size_t>>>& m_borders;

public:
    inline wm_dd_borders(const std::vector<std::vector<std::vector<size_t>>>& borders) 
        : m_borders(borders) {}

    inline size_t get_interval_start(size_t shard, size_t level, size_t interval) const {
        return interval == 0 ? 0 : m_borders[shard][level][interval - 1];
    }

    inline size_t get_interval_end(size_t shard, size_t level, size_t interval) const {
        return m_borders[shard][level][interval];
    }
};