#pragma once

#include <cstddef>

#include <vector/packed/packed_list.hpp>
#include <vector/packed/packed_util_simd.hpp>

#include <immintrin.h>

// 128bit-SSE-Specializations for packed_list read writes

template <>
inline __m128i packed_list<8, __m128i>::pack_aligned(size_t i, size_t n) const {        
    return mask_8(m_data[i / 16], n);
}

template <>
inline __m128i packed_list<8, __m128i>::pack(size_t i, size_t n) const {        
    size_t word = i / 16;
    size_t val = i % 16;

    assert(val + n <= 16);
    __m128i v = m_data[word];
    __m128i shifted = sr_8(v, val);
    return mask_8(shifted, n);
}

// AVX2

template <>
inline __m256i packed_list<8, __m256i>::pack_aligned(size_t i, size_t n) const {        
    return mask_8(m_data[i / 32], n);
}

template <>
inline __m256i packed_list<8, __m256i>::pack(size_t i, size_t n) const {        
    size_t word = i / 32;
    size_t val = i % 32;

    assert(val + n <= 32);
    __m256i v = m_data[word];
    __m256i shifted = sr_8(v, val);
    return mask_8(shifted, n);
}

// AVX-512

template <>
inline __m512i packed_list<8, __m512i>::pack_aligned(size_t i, size_t n) const {        
    return mask_8(m_data[i / 64], n);
}

template <>
inline __m512i packed_list<8, __m512i>::pack(size_t i, size_t n) const {        
    size_t word = i / 64;
    size_t val = i % 64;

    assert(val + n <= 64);
    __m512i v = m_data[word];
    __m512i shifted = sr_8(v, val);
    return mask_8(shifted, n);
}
