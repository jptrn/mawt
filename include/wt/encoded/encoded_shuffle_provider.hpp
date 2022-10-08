#pragma once

#include <util/bits.hpp>
#include <wt/encoded/packed_enc_data.hpp>
#include <wt/packed/packed_bit_util.hpp>
#include <wt/packed/packed_shuffle_provider.hpp>
#include <wt/packed/pshufb_shufmask_lut.hpp>
#include <wt/packed/pshufb_shufmask_128_lut.hpp>

#include <cstddef>

#include <immintrin.h>

#include <util/debug.hpp>

template <typename packed_enc_value_type>
inline packed_enc_data<packed_enc_value_type> split_enc_shuffle(packed_enc_value_type v, packed_enc_value_type l, size_t alpha, size_t count) {
    size_t bits = extract_8<packed_enc_value_type>(v, alpha, count);
    alpha = 8 - alpha - 1;

    // Split
    size_t right_count = __builtin_popcountll(bits);
    size_t left_count = count - right_count;
    
    const auto& shuffle_mask_lut = pshufb_shufmask_lut<packed_enc_value_type, sizeof(packed_enc_value_type)>::get();
    packed_enc_value_type shuffle_mask = shuffle_mask_lut.get_shuffle_mask(bits);

    packed_enc_value_type v_shuffled = (packed_enc_value_type)_mm_shuffle_pi8((__m64)v, (__m64)shuffle_mask);
    packed_enc_value_type l_shuffled = (packed_enc_value_type)_mm_shuffle_pi8((__m64)l, (__m64)shuffle_mask);

    packed_enc_value_type v_left = v_shuffled & mask(left_count * 8);
    packed_enc_value_type v_right = v_shuffled >> ((8 - right_count) * 8);

    packed_enc_value_type l_left = l_shuffled & mask(left_count * 8);
    packed_enc_value_type l_right = l_shuffled >> ((8 - right_count) * 8);

    return packed_enc_data<packed_enc_value_type>(left_count, right_count, v_left, v_right, l_left, l_right);
}

template <>
inline packed_enc_data<__m128i> split_enc_shuffle(__m128i v, __m128i l, size_t alpha, size_t count) {
    size_t bits = extract_8<__m128i>(v, alpha, count);
    alpha = 8 - alpha - 1;

    // Split
    size_t right_count = __builtin_popcountll(bits);
    size_t left_count = count - right_count;
    
    const auto& shuffle_mask_lut = pshufb_shufmask_128_lut<16>::get();
    __m128i shuffle_mask = shuffle_mask_lut.get_shuffle_mask(bits);

    __m128i v_shuffled = _mm_shuffle_epi8(v, shuffle_mask);
    __m128i l_shuffled = _mm_shuffle_epi8(l, shuffle_mask);

    __m128i v_left = mask_8(v_shuffled, left_count);
    __m128i v_right = sr_8(v_shuffled, 16 - right_count);

    __m128i l_left = mask_8(l_shuffled, left_count);
    __m128i l_right = sr_8(l_shuffled, 16 - right_count);

    return packed_enc_data<__m128i>(left_count, right_count, v_left, v_right, l_left, l_right);
}

template <>
inline packed_enc_data<__m256i> split_enc_shuffle(__m256i v, __m256i l, size_t alpha, size_t count) {
    // Pack
    size_t bits = extract_8(v, alpha, count);
    alpha = 8 - alpha - 1;

    // Split
    size_t right_count = __builtin_popcountll(bits);
    size_t left_count = count - right_count;
    
    __m256i v_left = _mm256_maskz_compress_epi8(~bits & mask(count), v);
    __m256i v_right = _mm256_maskz_compress_epi8(bits & mask(count), v);

    __m256i l_left = _mm256_maskz_compress_epi8(~bits & mask(count), l);
    __m256i l_right = _mm256_maskz_compress_epi8(bits & mask(count), l);

    return packed_enc_data<__m256i>(left_count, right_count, v_left, v_right, l_left, l_right);
}

template <>
inline packed_enc_data<__m512i> split_enc_shuffle(__m512i v, __m512i l, size_t alpha, size_t count) {
    // Pack
    size_t bits = extract_8(v, alpha, count);
    alpha = 8 - alpha - 1;

    // Split
    size_t right_count = __builtin_popcountll(bits);
    size_t left_count = count - right_count;
    
    __m512i v_left = _mm512_maskz_compress_epi8(~bits & mask(count), v);
    __m512i v_right = _mm512_maskz_compress_epi8(bits & mask(count), v);

    __m512i l_left = _mm512_maskz_compress_epi8(~bits & mask(count), l);
    __m512i l_right = _mm512_maskz_compress_epi8(bits & mask(count), l);

    return packed_enc_data<__m512i>(left_count, right_count, v_left, v_right, l_left, l_right);
}