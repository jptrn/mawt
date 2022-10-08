#pragma once

#include <util/bit_util_simd_lut.hpp>
#include <vector/packed/packed_util_simd.hpp>

#include <cstddef>

#include <immintrin.h>

// 128 Bits

// Masks out the upper `128 - n` bits and retains the lower `n` bits
__m128i mask_bits(__m128i val, size_t n) {
    if (n >= 128) {
        return val;
    }
    const auto& lut = bit_util_simd_lut<__m128i>::get();
    return _mm_and_si128(val, lut.mask(n));
}

// Masks out the lower `128 - n` bits and retains the upper `n` bits
__m128i mask_bits_lo(__m128i val, size_t n) {
    if (n >= 128) {
        return _mm_setzero_si128();
    }
    const auto& lut = bit_util_simd_lut<__m128i>::get();
    return _mm_andnot_si128(lut.mask(n), val);
}

__m128i sr_bits(__m128i val, size_t n) {
    // We cannot shift by bit amounts, but we can shift by bytes using
    // pshufb and friends and shift by bits using binary logic and maskking
    __m128i word_shift = sr_8(val, (n / 16) * 2);
    __m128i hi_bits = _mm_srli_epi16(word_shift, n % 16);
    __m128i lo_bits = _mm_slli_epi16(sr_8(word_shift, 2), 16 - (n % 16));
    return _mm_or_si128(hi_bits, lo_bits);
}

__m128i sl_bits(__m128i val, size_t n) {
    __m128i word_shift = sl_8(val, (n / 16) * 2);
    __m128i lo_bits = _mm_slli_epi16(word_shift, n % 16);
    __m128i hi_bits = _mm_srli_epi16(sl_8(word_shift, 2), 16 - (n % 16));
    return _mm_or_si128(hi_bits, lo_bits);
}

// 256 Bits

__m256i mask_bits(__m256i val, size_t n) {
    if (n >= 256) {
        return val;
    }
    const auto& lut = bit_util_simd_lut<__m256i>::get();
    return _mm256_and_si256(val, lut.mask(n));
}

// Masks out the lower `256 - n` bits and retains the upper `n` bits
__m256i mask_bits_lo(__m256i val, size_t n) {
    if (n >= 256) {
        return _mm256_setzero_si256();
    }
    const auto& lut = bit_util_simd_lut<__m256i>::get();
    return _mm256_andnot_si256(lut.mask(n), val);
}

__m256i sr_bits(__m256i val, size_t n) {
    __m256i word_shift = sr_8(val, (n / 16) * 2);
    __m256i hi_bits = _mm256_srli_epi16(word_shift, n % 16);
    __m256i lo_bits = _mm256_slli_epi16(sr_8(word_shift, 2), 16 - (n % 16));
    return _mm256_or_si256(hi_bits, lo_bits);
}

__m256i sl_bits(__m256i val, size_t n) {
    __m256i word_shift = sl_8(val, (n / 16) * 2);
    __m256i lo_bits = _mm256_slli_epi16(word_shift, n % 16);
    __m256i hi_bits = _mm256_srli_epi16(sl_8(word_shift, 2), 16 - (n % 16));
    return _mm256_or_si256(hi_bits, lo_bits);
}

// 512 Bits

__m512i mask_bits(__m512i val, size_t n) {
    if (n >= 512) {
        return val;
    }
    const auto& lut = bit_util_simd_lut<__m512i>::get();
    return _mm512_and_si512(val, lut.mask(n));
}

// Masks out the lower `512 - n` bits and retains the upper `n` bits
__m512i mask_bits_lo(__m512i val, size_t n) {
    if (n >= 512) {
        return _mm512_setzero_si512();
    }
    const auto& lut = bit_util_simd_lut<__m512i>::get();
    return _mm512_andnot_si512(lut.mask(n), val);
}

__m512i sr_bits(__m512i val, size_t n) {
    __m512i word_shift = sr_8(val, (n / 16) * 2);
    __m512i hi_bits = _mm512_srli_epi16(word_shift, n % 16);
    __m512i lo_bits = _mm512_slli_epi16(sr_8(word_shift, 2), 16 - (n % 16));
    return _mm512_or_si512(hi_bits, lo_bits);
}

__m512i sl_bits(__m512i val, size_t n) {
    __m512i word_shift = sl_8(val, (n / 16) * 2);
    __m512i lo_bits = _mm512_slli_epi16(word_shift, n % 16);
    __m512i hi_bits = _mm512_srli_epi16(sl_8(word_shift, 2), 16 - (n % 16));
    return _mm512_or_si512(hi_bits, lo_bits);
}
