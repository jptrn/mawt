#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

template <typename forward_iterable_type>
std::string join_to_string(const forward_iterable_type& t, const std::string& separator = ", ") {
    std::stringstream s;
    auto it = t.cbegin();
    if (it != t.cend()) {
        s << *it;
    }
    while (++it != t.cend()) {
        s << separator << *it;
    }
    return s.str();
}

template <typename value_type = uint64_t>
size_t count_alphabet(const std::vector<value_type>& text) {
    std::unordered_set<value_type> ch;
    for (auto s : text) {
        ch.emplace(s);
    }
    return ch.size();
}

template <typename value_type_in, typename value_type_out>
std::vector<value_type_out> remap_text(const std::vector<value_type_in>& text) {
    // Computes the effective alphabet of the text and remaps the text to the new alphabet
    // Writes the computed bit size into `new_depth`
    // The characters codes chosen for the remapped text are contiguous and maintain the same order between them as in the original text
    // e.g. 0 -> 0, 1 -> -, 2 -> -, 3 -> 1, etc.
    // Alternatively, it would also be possible to remap the characters by frequency, which could be interesting for compressed texts
    // I don't know if it has any implications for the standard case, though

    std::map<value_type_in, value_type_out> character_map;
    for (auto s : text) {
        character_map[s] = 0;
    }

    // Remap characters to consecutive indices
    value_type_out i = 0;
    for (auto& [ch, index] : character_map) {
        index = i++;
    }

    std::vector<value_type_out> remapped(text.size());
    for (size_t i = 0; i < remapped.size(); i++) {
        remapped[i] = character_map[text[i]];
    }
    return remapped;
}

template <typename value_type>
size_t read_file(const std::string& path, size_t alpha_bits, std::vector<value_type>& text, size_t length = 0) {
    std::ifstream file(path, std::ios::binary | std::ios::in);
    if (file.fail()) {
        return -1;
    }

    // Read file byte by byte and bit by bit
    // This can probably done more efficiently
    char cur_read;
    value_type cur_write = 0;
    uint64_t mask = 1ULL;
    while (file.get(cur_read) && (length == 0 || text.size() < length)) {
        for (size_t i = 0; i < 8; i++) {
            if (cur_read & 1) {
                cur_write |= mask;
            }
            cur_read >>= 1;
            mask <<= 1;
            if (mask >= (1ULL << alpha_bits)) {
                text.push_back(cur_write);
                mask = 1ULL;
                cur_write = 0;
            }
        }
    }
    if (mask != 1ULL) { // push back final (maybe incomplete) character
        text.push_back(cur_write);
    }

    return 0;
}

std::string get_timestamp(const std::string& fmt = "%Y-%m-%d-%H-%M-%S") {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::stringstream s;
    s << std::put_time(&tm, fmt.c_str());
    return s.str();
}

// creates a suffix array
template <typename value_type_in, typename value_type_out>
std::vector<value_type_out> create_suffix_array(std::vector<value_type_in> text) {
    std::vector<value_type_out> suffix_array(text.size());
    std::iota(suffix_array.begin(), suffix_array.end(), 0); 
    std::sort(suffix_array.begin(), suffix_array.end(), [&text](size_t a, size_t b) {
        while (true) {
            if (a >= text.size() || b >= text.size()) {
                return a > b;
            } else if (text[a] == text[b]) {
                a++; 
                b++;
            } else {
                return text[a] < text[b];
            }
        }
    });

    return suffix_array;
}

template <typename value_type>
double get_h0(const std::vector<value_type>& text) {
    // First, get the probabilities of each character
    std::unordered_map<value_type, size_t> hist;
    for (auto c : text) {
        hist[c]++;
    }

    // Now accumulate to get the entropy
    double h0 = 0.0;
    double n = text.size();
    for (const auto& [ ch, nc ] : hist) {
        h0 += (nc / n) * log2(n / nc);
    }

    return h0;
}