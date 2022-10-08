#pragma once

#include <cstddef>
#include <iostream>
#include <vector>

#include <util/bits.hpp>

#include <benchmark_wt/benchmark_params.hpp>

// Remaps a text
template <typename benchmark_type, typename value_type_raw>
class benchmark_text_processor {
private:
    const benchmark_type& m_benchmark;

    template <typename value_type>
    void remap_and_run(const std::vector<value_type_raw>& text, size_t alpha_size, size_t alpha_bits) {        
        auto remapped = remap_text<value_type_raw, value_type>(text);
        run(remap_text<value_type_raw, value_type>(text), alpha_size, alpha_bits);
    }

    template <typename value_type>
    void run(const std::vector<value_type>& text, size_t alpha_size, size_t alpha_bits) {        
        std::cout << "Text width: " << sizeof(value_type) << " byte(s) (was " << sizeof(value_type_raw) << " byte(s))" << std::endl;
        m_benchmark.run(text, alpha_size, alpha_bits);
    }

    void remap_and_run(const std::vector<value_type_raw>& text_raw, size_t alpha_size, size_t alpha_bits) {
        if (alpha_bits <= 8) {
            remap_and_run<uint8_t>(text_raw, alpha_size, alpha_bits);
        } else if (alpha_bits <= 16) {
            remap_and_run<uint16_t>(text_raw, alpha_size, alpha_bits);
        } else if (alpha_bits <= 32) {
            remap_and_run<uint32_t>(text_raw, alpha_size, alpha_bits);
        } else if (alpha_bits <= 64) {
            remap_and_run<uint64_t>(text_raw, alpha_size, alpha_bits);
        }
    }

public:
    benchmark_text_processor(const benchmark_type& benchmark) : m_benchmark(benchmark) {}

    void process_text_and_run(const benchmark_params& params, const std::vector<value_type_raw>& text_raw, const size_t alpha_bits) {
        if (params.compute_effective_alphabet) {
            size_t real_alpha_size = count_alphabet(text_raw);
            size_t real_alpha_bits = log2_ceil(real_alpha_size);
            std::cout << "Computed effective alphabet of text, " << real_alpha_size << " characters, depth: " << real_alpha_bits << std::endl;
            remap_and_run(text_raw, real_alpha_size, real_alpha_bits);
        } else {
            size_t alpha_size = 1ULL << alpha_bits;
            std::cout << "Alphabet of text " << alpha_size << " characters, depth: " << alpha_bits << std::endl;
            run(text_raw, alpha_size, alpha_bits);
        }
    }
};



