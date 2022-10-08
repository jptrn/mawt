#pragma once

#include <cstdint>
#include <cstddef>
#include <limits>
#include <random>
#include <string>

#include <util/random.hpp>

#include <benchmark_wt/benchmark_params.hpp>

// Provides access to random numbers
class benchmark_rng {
private:
    const benchmark_params& m_params;

    // RNGs
    random_number_generator<uint64_t, std::uniform_int_distribution<uint64_t>> m_rng_uniform;
    size_t m_seed;
    
public:
    using result_type = size_t;

    benchmark_rng(const benchmark_params& params) : m_params(params) {
        if (params.distribution == "uniform") {
            if (params.random_seed) {
                m_rng_uniform = random_number_generator<>();
                m_seed = m_rng_uniform.seed();
            } else {
                m_seed = params.seed;
                m_rng_uniform = random_number_generator<>(m_seed);
            }
        }
    }

    size_t next() {
        // Todo: If multiple distribution types are added, this will have to be checked
        if (m_params.distribution == "uniform") {
            return m_rng_uniform.next();
        } else {
            return 0;
        }
    }

    size_t operator()() {
        return next();
    }

    size_t min() const {
        return std::numeric_limits<size_t>::min();
    }

    size_t max() const {
        return std::numeric_limits<size_t>::max();
    }

    size_t seed() const {
        return m_seed;
    }
};