#pragma once

#include <cstddef>
#include <iostream>
#include <vector>
#include <unordered_map>

#include <util/bits.hpp>

#include <benchmark_util.hpp>
#include <benchmark_wt/benchmark_params.hpp>

// Remaps a text
template <typename benchmark_type, typename value_type_raw>
class benchmark_remap_words {
private:
    const benchmark_type& m_benchmark;

    template <typename value_type>
    void run(const std::vector<size_t>& word_text) {
        std::cout << "Text width: " << sizeof(value_type) << " byte(s)" << std::endl;
        std::vector<value_type> remapped;
        remapped.reserve(word_text.size());
        for (auto c : word_text) {
            remapped.push_back(c);
        }
        m_benchmark.run(remapped, remapped.size(), log2_ceil(remapped.size()));
    }

    std::vector<size_t> count_words(const std::vector<value_type_raw>& text_raw, size_t sep) {
        std::vector<size_t> new_text;
        std::unordered_map<std::string, size_t> words;

        std::string cur_word = "";
        for (auto c : text_raw) {
            if (isspace(c) || c == sep) { // TODO - when do I split words?
                size_t new_idx = 0;
                if (!words.contains(cur_word)) {
                    new_idx = new_text.size();
                    words.emplace(cur_word, new_idx);
                } else {
                    new_idx = words[cur_word];
                }

                
                new_text.push_back(new_idx);    
                cur_word = "";
            } else {
                cur_word.push_back(c);
            }
        }

        return new_text;
    }

public:
    benchmark_remap_words(const benchmark_type& benchmark) : m_benchmark(benchmark) {}

    void run(const std::vector<value_type_raw>& text_raw, size_t sep) {
        auto words = count_words(text_raw, sep);
        auto remap_words_bits = log2_ceil(*std::max(words.begin(), words.end()));
        if (remap_words_bits <= 8) {
            run<uint8_t>(words);
        } else if (remap_words_bits <= 16) {
            run<uint16_t>(words);
        } else if (remap_words_bits <= 32) {
            run<uint32_t>(words);
        } else if (remap_words_bits <= 64) {
            run<uint64_t>(words);
        }
    }
};



