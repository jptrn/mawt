#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <benchmark_wt/benchmark_execution.hpp>
#include <benchmark_wt/benchmark_params.hpp>
#include <benchmark_wt/benchmark_remap_words.hpp>
#include <benchmark_wt/benchmark_results.hpp>
#include <benchmark_wt/benchmark_suffix_array.hpp>
#include <benchmark_wt/benchmark_text_processor.hpp>

// Reads a text from file and starts `params.iterations` benchmark iterations with it
// `value_type_raw` is the type that is read from file. After (optional) character remapping,
// it is converted to `value_type`
template <typename value_type_raw>
class benchmark_read_file {
private:
    const benchmark_params& m_params;
    benchmark_results& m_results;
    const std::string& m_path;
    size_t m_alpha_bits;

public:
    benchmark_read_file(const benchmark_params& params, benchmark_results& results, const std::string& path, size_t alpha_bits) :
        m_params(params),
        m_results(results),
        m_path(path),
        m_alpha_bits(alpha_bits) {};

    void run() {
        // Read raw text from file
        std::cout << "Reading text from " << m_path << "..." << std::endl;
        std::cout << "Reading " << m_alpha_bits << " bits" << std::endl;

        std::vector<value_type_raw> text_raw;

        if (read_file(m_path, m_alpha_bits, text_raw, m_params.text_min)) {
            std::cout << "Error reading file '" << m_path << "'!" << std::endl;
            std::exit(EXIT_FAILURE);
        }
        std::cout << "Text size: " << text_raw.size() << std::endl;

        // Remap alphabet and run
        if (m_params.create_suffix_array) {
            std::cout << "Creating suffix array..." << std::endl;
            benchmark_suffix_array<benchmark_read_file<value_type_raw>, value_type_raw>(*this).run(text_raw);
        } else if (m_params.remap_to_words) {
            std::cout << "Find words in text" << std::endl;
            benchmark_remap_words<benchmark_read_file<value_type_raw>, value_type_raw>(*this).run(text_raw, m_params.remap_to_words_sep);
        } else {
            benchmark_text_processor<benchmark_read_file<value_type_raw>, value_type_raw>(*this).process_text_and_run(m_params, text_raw, m_alpha_bits);
        }
        std::cout << std::endl;
    }

    // Template callback for `benchmark_text_processor`, executes the benchmark with the remapped text
    template <typename value_type>
    void run(const std::vector<value_type>& text, size_t alpha_size, size_t alpha_bits) const {        
        for (size_t iteration = 0; iteration < m_params.iterations; iteration++) {
            std::cout << "Iteration " << iteration << "..." << std::endl;
            benchmark_execution(m_params, m_results, text, alpha_size, alpha_bits, iteration, m_path).run();
            std::cout << std::endl;
        }
    }
};

void bench_read_file(const benchmark_params& params, benchmark_results& results, const std::string& path, size_t alpha_bits) {
    // Clumsy, but the only real solution i can see 
    if (alpha_bits <= 8) {
        benchmark_read_file<uint8_t>(params, results, path, alpha_bits).run();
    } else if (alpha_bits <= 16) {
        benchmark_read_file<uint16_t>(params, results, path, alpha_bits).run();
    } else if (alpha_bits <= 32) {
        benchmark_read_file<uint32_t>(params, results, path, alpha_bits).run();
    } else if (alpha_bits <= 64) {
        benchmark_read_file<uint64_t>(params, results, path, alpha_bits).run();
    }
}
