#pragma once

#include <cstddef>
#include <tuple>

#include <vector/encoded/encoded_vector.hpp>
#include <vector/packed/packed_util_simd.hpp>

#include <immintrin.h>

// 128bit-SSE-Specializations for encoded_vector read writes

template <>
inline std::tuple<__m128i, __m128i> encoded_vector<__m128i, 8>::get(size_t i, size_t n) const {        
    size_t word = i / 16;
    size_t val = i % 16;

    assert(val + n <= 16);
    __m128i v = mask_8(sr_8(m_data[word], val), n);
    __m128i l = mask_8(sr_8(m_sizes[word], val), n);
    return { v, l };
}

// AVX2

template <>
inline std::tuple<__m256i, __m256i> encoded_vector<__m256i, 8>::get(size_t i, size_t n) const {        
    size_t word = i / 32;
    size_t val = i % 32;

    assert(val + n <= 32);
    __m256i v = mask_8(sr_8(m_data[word], val), n);
    __m256i l = mask_8(sr_8(m_sizes[word], val), n);
    return { v, l };
}


// AVX-512

template <>
inline std::tuple<__m512i, __m512i> encoded_vector<__m512i, 8>::get(size_t i, size_t n) const {        
    size_t word = i / 64;
    size_t val = i % 64;

    assert(val + n <= 64);
    __m512i v = mask_8(sr_8(m_data[word], val), n);
    __m512i l = mask_8(sr_8(m_sizes[word], val), n);
    return { v, l };
}