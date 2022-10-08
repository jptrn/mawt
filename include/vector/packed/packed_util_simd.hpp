#pragma once

#include <vector/packed/packed_util_simd_lut.hpp>
#include <util/bits.hpp>

#include <cstddef>

#include <immintrin.h>

// SIMD-reimplementations of operations I need in scalar space
// For SIMD, we require that packed values are whole bytes/shorts/etc., since
// that allows us to use pshufb and pre-generated shuffle masks as a catch-all
// method for all operations I need

// Masks out the upper `16 - n` bytes and retains the lower `n` bytes
__m128i mask_8(__m128i val, size_t n) {
    if (n >= 16) {
        return val;
    }
    const auto& lut = packed_util_simd_lut<__m128i, uint8_t>::get();
    return _mm_and_si128(lut.mask(n), val);
}

// Masks out the lower `16 - n` bytes and retains the upper `n` bytes
__m128i mask_lo_8(__m128i val, size_t n) {
    if (n >= 16) {
        return _mm_setzero_si128();
    }
    const auto& lut = packed_util_simd_lut<__m128i, uint8_t>::get();
    return _mm_andnot_si128(lut.mask(n), val);
}

// Shifts the value right by `n` bytes
__m128i sr_8(__m128i val, size_t n) {
    if (n >= 16) {
        return _mm_setzero_si128();
    }
    const auto& lut = packed_util_simd_lut<__m128i, uint8_t>::get();
    return _mm_shuffle_epi8(val, lut.sr(n));
}

// Shifts the value left by `n` bytes
__m128i sl_8(__m128i val, size_t n) {
    if (n >= 16) {
        return _mm_setzero_si128();
    }
    const auto& lut = packed_util_simd_lut<__m128i, uint8_t>::get();
    return _mm_shuffle_epi8(val, lut.sl(n));
}

// AVX-512

__m256i mask_8(__m256i val, size_t n) {
    if (n >= 32) {
        return val;
    }
    const auto& lut = packed_util_simd_lut<__m256i, uint8_t>::get();
    return _mm256_and_si256(lut.mask(n), val);
}

// Masks out the lower `16 - n` bytes and retains the upper `n` bytes
__m256i mask_lo_8(__m256i val, size_t n) {
    if (n >= 32) {
        return _mm256_setzero_si256();
    }
    const auto& lut = packed_util_simd_lut<__m256i, uint8_t>::get();
    return _mm256_andnot_si256(lut.mask(n), val);
}

__m256i sl_8(__m256i val, size_t n) {
    if (n >= 32) {
        return _mm256_setzero_si256();
    }
    const auto& lut = packed_util_simd_lut<__m256i, uint8_t>::get();
    // we can't cross 128-bit lane boundaries with pshufb, but we can with vperm
    // I hope this isn't too bad for performance
    return _mm256_maskz_permutexvar_epi8(0xFFFFFFFFULL << n, lut.sl(n), val);
}

__m256i sr_8(__m256i val, size_t n) {
    if (n >= 32) {
        return _mm256_setzero_si256();
    }
    const auto& lut = packed_util_simd_lut<__m256i, uint8_t>::get();
    return _mm256_maskz_permutexvar_epi8(0xFFFFFFFFULL >> n, lut.sr(n), val);
}


// AVX-512

__m512i mask_8(__m512i val, size_t n) {
    if (n >= 64) {
        return val;
    }
    const auto& lut = packed_util_simd_lut<__m512i, uint8_t>::get();
    return _mm512_and_si512(lut.mask(n), val);
}

// Masks out the lower `16 - n` bytes and retains the upper `n` bytes
__m512i mask_lo_8(__m512i val, size_t n) {
    if (n >= 64) {
        return _mm512_setzero_si512();
    }
    const auto& lut = packed_util_simd_lut<__m512i, uint8_t>::get();
    return _mm512_andnot_si512(lut.mask(n), val);
}

__m512i sl_8(__m512i val, size_t n) {
    if (n >= 64) {
        return _mm512_setzero_si512();
    }
    const auto& lut = packed_util_simd_lut<__m512i, uint8_t>::get();
    return _mm512_maskz_permutexvar_epi8(-1ULL << n, lut.sl(n), val);
}

__m512i sr_8(__m512i val, size_t n) {
    if (n >= 64) {
        return _mm512_setzero_si512();
    }
    const auto& lut = packed_util_simd_lut<__m512i, uint8_t>::get();
    // we can't cross 128-bit lane boundaries with pshufb, but we can with vperm
    // I hope this isn't too bad for performance
    return _mm512_maskz_permutexvar_epi8(-1ULL >> n, lut.sr(n), val);
}
