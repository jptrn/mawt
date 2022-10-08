#pragma once

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <numeric>
#include <iostream>
#include <vector>

#include <benchmark_wt/benchmark_execution.hpp>
#include <benchmark_wt/benchmark_params.hpp>
#include <benchmark_wt/benchmark_results.hpp>
#include <benchmark_wt/benchmark_rng.hpp>
#include <benchmark_wt/benchmark_text_processor.hpp>

// Generates a text and runs a benchmark on it
// `value_type_raw` is the type that is read from file. After (optional) character remapping,
// it is converted to `value_type`
template <typename value_type_raw>
class benchmark_random {
private:
    const benchmark_params& m_params;
    benchmark_results& m_results;
    benchmark_rng& m_rng;
    size_t m_size;
    size_t m_alpha_bits;
    size_t m_iteration;

public:
    benchmark_random(const benchmark_params& params, benchmark_results& results, benchmark_rng& rng, size_t text_size, size_t alpha_bits, size_t iteration) :
        m_params(params),
        m_results(results),
        m_rng(rng),
        m_size(text_size),
        m_alpha_bits(alpha_bits),
        m_iteration(iteration) {};

    void run() {
        std::cout << "Generating random text with size " << m_size << ", alphabet size: " << (1ULL << m_alpha_bits) << " using distribution: '" << m_params.distribution << "'..." << std::endl;
        std::vector<value_type_raw> text_raw;
        if (m_params.create_suffix_array) {
            // If we do not provide `--file ...` parameters but this option is checked, we just create a permutation of 1..n
            text_raw.resize(m_size);
            std::iota(text_raw.begin(), text_raw.end(), 0);
            std::shuffle(text_raw.begin(), text_raw.end(), m_rng);
        } else {
            text_raw.reserve(m_size);
            for (size_t i = 0; i < m_size; i++) {
                auto generated = m_rng.next() & mask(m_alpha_bits);
                text_raw.push_back(generated);
            }
        }

        // Remap alphabet and run
        benchmark_text_processor<benchmark_random<value_type_raw>, value_type_raw>(*this).process_text_and_run(m_params, text_raw, m_alpha_bits);
        std::cout << std::endl;
    }

    // Template callback for `benchmark_text_processor`, executes the benchmark with the remapped text
    template <typename value_type>
    void run(const std::vector<value_type>& text, size_t alpha_size, size_t alpha_bits) const {        
        benchmark_execution(m_params, m_results, text, alpha_size, alpha_bits, m_iteration).run();
    }
};

void bench_random(const benchmark_params& params, benchmark_results& results, benchmark_rng& rng, size_t iteration, size_t text_size, size_t alpha_bits) {
    // Clumsy, but the only real solution i can see 
    size_t size_bits = params.create_suffix_array ? log2_ceil(text_size) : alpha_bits;
    if (size_bits <= 8) {
        benchmark_random<uint8_t>(params, results, rng, text_size, size_bits, iteration).run();
    } else if (size_bits <= 16) {
        benchmark_random<uint16_t>(params, results, rng, text_size, size_bits, iteration).run();
    } else if (size_bits <= 32) {
        benchmark_random<uint32_t>(params, results, rng, text_size, size_bits, iteration).run();
    } else if (size_bits <= 64) {
        benchmark_random<uint64_t>(params, results, rng, text_size, size_bits, iteration).run();
    }
}