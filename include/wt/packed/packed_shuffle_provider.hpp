#pragma once

#include <util/bits.hpp>
#include <vector/packed/packed_util_simd.hpp>
#include <wt/packed/packed_bit_util.hpp>
#include <wt/packed/packed_bit_util_simd.hpp>
#include <wt/packed/packed_data.hpp>
#include <wt/packed/pshufb_shufmask_lut.hpp>
#include <wt/packed/pshufb_shufmask_128_lut.hpp>

#include <cstddef>
#include <cstdint>

#include <immintrin.h>

template <typename packed_value_type>
inline size_t extract_8(packed_value_type v, size_t alpha, size_t count) {
    alpha = 8 - alpha - 1;
    size_t l = get_l<packed_value_type, 8>();
    return _pext_u64(v & mask(8 * count), l << alpha);
}

template <>
inline size_t extract_8(__m128i v, size_t alpha, size_t count) {
    __m128i shufmask = bitshuf_mask_128_8[8 - alpha - 1];
    __mmask16 shuffled = _mm_bitshuffle_epi64_mask(v, shufmask);
    return shuffled & mask(count);

    // Non AVX-512 instruction
    // __m128i bit = bit_n_128_8[8 - alpha - 1];
    // __m128i masked = mask_8(v, count);
    // __m128i packed_bits = pext_128(masked, bit);
    // For performance, it might be better to directly write the m128i to memory
    // return _mm_extract_epi64(packed_bits, 0);   
}

template <>
inline size_t extract_8(__m256i v, size_t alpha, size_t count) {
    __m256i shufmask = bitshuf_mask_256_8[8 - alpha - 1];
    __mmask32 shuffled = _mm256_bitshuffle_epi64_mask(v, shufmask);
    return shuffled & mask(count);
}

template <>
inline size_t extract_8(__m512i v, size_t alpha, size_t count) {
    __m512i shufmask = bitshuf_mask_512_8[8 - alpha - 1];
    __mmask64 shuffled = _mm512_bitshuffle_epi64_mask(v, shufmask);
    return shuffled & mask(count);
}

template <typename packed_value_type>
inline packed_data<packed_value_type> get_packed_data_shuffle(packed_value_type v, size_t alpha, size_t count) {
    // Pack
    size_t bits = extract_8(v, alpha, count);
    alpha = 8 - alpha - 1;

    // Split
    size_t right_count = __builtin_popcountll(bits);
    size_t left_count = count - right_count;
    
    const auto& shuffle_mask_lut = pshufb_shufmask_lut<packed_value_type, 8>::get();
    packed_value_type shuffle_mask = shuffle_mask_lut.get_shuffle_mask(bits);
    packed_value_type shuffled = (packed_value_type)_mm_shuffle_pi8((__m64)v, (__m64)shuffle_mask);

    packed_value_type left = shuffled & mask(left_count * 8);
    packed_value_type right = shuffled >> ((8 - right_count) * 8);

    return packed_data<packed_value_type>(left_count, right_count, left, right, bits);
}

template <>
inline packed_data<__m128i> get_packed_data_shuffle(__m128i v, size_t alpha, size_t count) {
    // Pack
    size_t bits = extract_8(v, alpha, count);
    alpha = 8 - alpha - 1;

    // Split
    size_t right_count = __builtin_popcountll(bits);
    size_t left_count = count - right_count;
    
    const auto& shuffle_mask_lut = pshufb_shufmask_128_lut<16>::get();
    __m128i shuffle_mask = shuffle_mask_lut.get_shuffle_mask(bits);
    __m128i shuffled = _mm_shuffle_epi8(v, shuffle_mask);

    __m128i left = mask_8(shuffled, left_count);
    __m128i right = sr_8(shuffled, 16 - right_count);

    return packed_data<__m128i>(left_count, right_count, left, right, bits);
}

template <>
inline packed_data<__m256i> get_packed_data_shuffle(__m256i v, size_t alpha, size_t count) {
    // Pack
    size_t bits = extract_8(v, alpha, count);
    alpha = 8 - alpha - 1;

    // Split
    size_t right_count = __builtin_popcountll(bits);
    size_t left_count = count - right_count;
    
    // Thank you, StackOverflow
    __m256i left = _mm256_maskz_compress_epi8(~bits & mask(count), v);
    __m256i right = _mm256_maskz_compress_epi8(bits & mask(count), v);

    return packed_data<__m256i>(left_count, right_count, left, right, bits);
}

template <>
inline packed_data<__m512i> get_packed_data_shuffle(__m512i v, size_t alpha, size_t count) {
    // Pack
    size_t bits = extract_8(v, alpha, count);
    alpha = 8 - alpha - 1;

    // Split
    size_t right_count = __builtin_popcountll(bits);
    size_t left_count = count - right_count;
    
    __m512i left = _mm512_maskz_compress_epi8(~bits & mask(count), v);
    __m512i right = _mm512_maskz_compress_epi8(bits & mask(count), v);

    return packed_data<__m512i>(left_count, right_count, left, right, bits);
}

template <typename packed_value_type> 
inline size_t count_zeros_8(packed_value_type v, size_t alpha, size_t count) {
    auto c = check0<packed_value_type, 8>(v, 8 - alpha - 1);      
    return __builtin_popcountll(c & mask(8 * count));  
}

template <> 
inline size_t count_zeros_8(__m128i v, size_t alpha, size_t count) {
    return count - __builtin_popcountll(extract_8(v, alpha, count));
}

template <> 
inline size_t count_zeros_8(__m256i v, size_t alpha, size_t count) {
    return count - __builtin_popcountll(extract_8(v, alpha, count));
}

template <> 
inline size_t count_zeros_8(__m512i v, size_t alpha, size_t count) {
    return count - __builtin_popcountll(extract_8(v, alpha, count));
}