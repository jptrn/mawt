#pragma once

#include <cstddef>
#include <iostream>
#include <vector>

#include <util/bits.hpp>

#include <benchmark_util.hpp>
#include <benchmark_wt/benchmark_params.hpp>

// Remaps a text
template <typename benchmark_type, typename value_type_raw>
class benchmark_suffix_array {
private:
    const benchmark_type& m_benchmark;

    template <typename value_type>
    void create_suffix_array_and_run(const std::vector<value_type_raw>& text) {        
        auto suffix_array = create_suffix_array<value_type_raw, value_type>(text);
        std::cout << "Text width: " << sizeof(value_type) << " byte(s)" << std::endl;
        m_benchmark.run(suffix_array, suffix_array.size(), log2_ceil(suffix_array.size()));
    }

public:
    benchmark_suffix_array(const benchmark_type& benchmark) : m_benchmark(benchmark) {}

    void run(const std::vector<value_type_raw>& text_raw) {
        size_t suffix_array_bits = log2_ceil(text_raw.size());
        if (suffix_array_bits <= 8) {
            create_suffix_array_and_run<uint8_t>(text_raw);
        } else if (suffix_array_bits <= 16) {
            create_suffix_array_and_run<uint16_t>(text_raw);
        } else if (suffix_array_bits <= 32) {
            create_suffix_array_and_run<uint32_t>(text_raw);
        } else if (suffix_array_bits <= 64) {
            create_suffix_array_and_run<uint64_t>(text_raw);
        }
    }
};



