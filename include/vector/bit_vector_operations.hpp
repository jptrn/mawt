#pragma once

#include <vector/bit_vector.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <sstream>
#include <vector>

// Operations on a bit vector, which aren't part of the class itself

// -- Naive rank implementations --

inline size_t bit_vector_rank1(const bit_vector& bv, size_t start, size_t end) {
    if (end < start) {
        return 0;
    }
    size_t rank = 0;
    for (size_t i = start; i <= end; i++) {
        rank += bv[i];
    }
    return rank;
}

inline size_t bit_vector_rank1(const bit_vector& bv, size_t k) {
    return bit_vector_rank1(bv, 0, k);
}

inline size_t bit_vector_rank1(const bit_vector& bv) {
    return bit_vector_rank1(bv, bv.size() - 1);
}

inline size_t bit_vector_rank0(const bit_vector& bv, size_t start, size_t end) {
    return (end - start + 1) - bit_vector_rank1(bv, start, end);
}

inline size_t bit_vector_rank0(const bit_vector& bv, size_t k) {
    return bit_vector_rank0(bv, 0, k);
}

inline size_t bit_vector_rank0(const bit_vector& bv) {
    return bit_vector_rank0(bv, 0, bv.size() - 1);
}


// -- To Value --

template <typename value_type>
constexpr static size_t max_bit_size() {
    return sizeof(value_type) * 8;
}

template <typename value_type>
value_type bit_vector_to(const bit_vector& bv, size_t max_bits = max_bit_size<value_type>()) {
    value_type bits = 0;
    size_t mask = 1ULL;
    for (size_t i = 0; i < std::min(max_bits, bv.size()); i++) {
        bits |= bv[i] ? mask : 0;
        mask <<= 1;
    }
    return bits;
}

// -- To String / From String --

inline std::string bit_vector_to_string(const bit_vector& bv, char t = '1', char f = '0', bool reverse = false) {
    std::stringstream s;
    if (!reverse) {
        for (size_t i = 0; i < bv.size(); i++) {
            s << (bv[i] ? t : f);
        }
    } else {
        for (size_t i = bv.size() - 1; i != -1ULL; i--) {
            s << (bv[i] ? t : f);
        }
    }
    return s.str();
}

inline bit_vector bit_vector_from_string(const std::string& str, char t = '1', char f = '0', bool reverse = false) {
    std::vector<bool> bits;
    bits.reserve(str.length());
    if (!reverse) {
        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == t) {
                bits.push_back(true);
            } else if (str[i] == f) {
                bits.push_back(false);
            }
        }
    } else {
        for (size_t i = str.length() - 1; i != -1ULL; i--) {
            if (str[i] == t) {
                bits.push_back(true);
            } else if (str[i] == f) {
                bits.push_back(false);
            }
        }
    }
    return bit_vector(bits);
}
