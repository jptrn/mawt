#pragma once

#include <wt/packed/packed_bit_util.hpp>
#include <wt/encoded/encoded_compress_shufmask_lut.hpp>
#include <wt/encoded/encoded_compress_shufmask_128_lut.hpp>
#include <wt/packed/packed_shuffle_provider.hpp>

#include <cstddef>
#include <tuple>

#include <immintrin.h>

template <typename packed_enc_value_type>
inline size_t get_compress_mask_shuffle(packed_enc_value_type l, size_t alpha, size_t count) {
    __m64 cmp = _mm_cmpgt_pi8((__m64)l, _mm_set1_pi8(alpha & 7));
    return extract_8<packed_enc_value_type>((packed_enc_value_type)cmp, 0, count);
}

template <typename packed_enc_value_type, size_t N>
inline std::tuple<packed_enc_value_type, packed_enc_value_type, size_t> compress_values_shuffle(packed_enc_value_type v, packed_enc_value_type l, size_t alpha, size_t count) {
    size_t bits = get_compress_mask_shuffle<packed_enc_value_type>(l, alpha, count);
    size_t new_count = __builtin_popcountll(bits);
    const auto& shuffle_mask_lut = encoded_compress_shufmask_lut<packed_enc_value_type, N>::get();
    packed_enc_value_type mask = shuffle_mask_lut.get_shuffle_mask(bits);
    __m64 compressed_v = _mm_shuffle_pi8((__m64)v, (__m64)mask);
    __m64 compressed_l = _mm_shuffle_pi8((__m64)l, (__m64)mask);
    return { (size_t)compressed_v, (size_t)compressed_l, new_count };
}


template <>
inline size_t get_compress_mask_shuffle<__m128i>(__m128i l, size_t alpha, size_t count) {
    __m128i cmp = _mm_cmpgt_epi8(l, _mm_set1_epi8(alpha & 7));
    return extract_8<__m128i>(cmp, 0, count);
}

template <>
inline std::tuple<__m128i, __m128i, size_t> compress_values_shuffle<__m128i, 16>(__m128i v, __m128i l, size_t alpha, size_t count) {
    size_t bits = get_compress_mask_shuffle<__m128i>(l, alpha, count);
    size_t new_count = __builtin_popcountll(bits);
    const auto& shuffle_mask_lut = encoded_compress_shufmask_128_lut<16>::get();
    __m128i mask = shuffle_mask_lut.get_shuffle_mask(bits);
    __m128i compressed_v = _mm_shuffle_epi8(v, mask);
    __m128i compressed_l = _mm_shuffle_epi8(l, mask);
    return { compressed_v, compressed_l, new_count };
}

template <>
inline size_t get_compress_mask_shuffle<__m256i>(__m256i l, size_t alpha, size_t count) {
    __m256i cmp = _mm256_cmpgt_epi8(l, _mm256_set1_epi8(alpha & 7));
    return extract_8<__m256i>(cmp, 0, count);
}

template <>
inline std::tuple<__m256i, __m256i, size_t> compress_values_shuffle<__m256i, 32>(__m256i v, __m256i l, size_t alpha, size_t count) {
    size_t bits = get_compress_mask_shuffle<__m256i>(l, alpha, count);
    size_t new_count = __builtin_popcountll(bits);
    __m256i compressed_v = _mm256_maskz_compress_epi8(bits, v);
    __m256i compressed_l = _mm256_maskz_compress_epi8(bits, l);
    return { compressed_v, compressed_l, new_count };
}

template <>
inline size_t get_compress_mask_shuffle<__m512i>(__m512i l, size_t alpha, size_t count) {
    return _mm512_cmpgt_epi8_mask(l, _mm512_set1_epi8(alpha & 7)) & mask(count);
}

template <>
inline std::tuple<__m512i, __m512i, size_t> compress_values_shuffle<__m512i, 64>(__m512i v, __m512i l, size_t alpha, size_t count) {
    size_t bits = get_compress_mask_shuffle<__m512i>(l, alpha, count);
    size_t new_count = __builtin_popcountll(bits);
    __m512i compressed_v = _mm512_maskz_compress_epi8(bits, v);
    __m512i compressed_l = _mm512_maskz_compress_epi8(bits, l);
    return { compressed_v, compressed_l, new_count };
}
