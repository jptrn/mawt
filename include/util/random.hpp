#pragma once

#include <cstdint>
#include <random>

template <typename size_type = uint64_t, typename distribution_type = std::uniform_int_distribution<size_type>>
class random_number_generator {
private:
    std::random_device::result_type m_seed;
    std::mt19937 m_generator;
    distribution_type m_dist;

    inline static std::random_device::result_type random_seed() {
        static std::random_device rd;
        return rd();
    }

public:
    inline random_number_generator() : random_number_generator(random_seed()) {}

    inline random_number_generator(std::random_device::result_type seed) : m_seed(seed), m_generator(seed) {}

    inline size_type next() {
        return m_dist(m_generator);
    }
    
    inline std::random_device::result_type seed() const {
        return m_seed;
    }
};