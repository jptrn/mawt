#include <gtest/gtest.h>

#include <util/bit_util_simd.hpp>

#include <cstddef>
#include <cstdint>

#include <immintrin.h>

#include "util/simd_util.hpp"

TEST(Simd128, Mask) {
    __m128i val = _mm_set_epi8(0xFF, 0x32, 0x17, 0x32, 0x00, 0x19, 0x32, 0x50, 0x17, 0xFE, 0x12, 0x06, 0x35, 0x84, 0x81, 0xFB);
    ASSERT_SIMD_EQ(mask_bits(val, 0),    _mm_set_epi8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));
    ASSERT_SIMD_EQ(mask_bits(val, 1),    _mm_set_epi8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01));
    ASSERT_SIMD_EQ(mask_bits(val, 5),    _mm_set_epi8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B));
    ASSERT_SIMD_EQ(mask_bits(val, 8),    _mm_set_epi8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFB));
    ASSERT_SIMD_EQ(mask_bits(val, 10),   _mm_set_epi8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFB));
    ASSERT_SIMD_EQ(mask_bits(val, 29),   _mm_set_epi8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x84, 0x81, 0xFB));
    ASSERT_SIMD_EQ(mask_bits(val, 64),   _mm_set_epi8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0xFE, 0x12, 0x06, 0x35, 0x84, 0x81, 0xFB));
    ASSERT_SIMD_EQ(mask_bits(val, 126),  _mm_set_epi8(0x3F, 0x32, 0x17, 0x32, 0x00, 0x19, 0x32, 0x50, 0x17, 0xFE, 0x12, 0x06, 0x35, 0x84, 0x81, 0xFB));
    ASSERT_SIMD_EQ(mask_bits(val, 127),  _mm_set_epi8(0x7F, 0x32, 0x17, 0x32, 0x00, 0x19, 0x32, 0x50, 0x17, 0xFE, 0x12, 0x06, 0x35, 0x84, 0x81, 0xFB));
    for (size_t i = 128; i < 256; i++) {
        ASSERT_SIMD_EQ(mask_bits(val, i), val);
    }
}

TEST(Simd128, MaskLo) {
    __m128i val = _mm_set_epi8(0xFF, 0x32, 0x17, 0x32, 0x00, 0x19, 0x32, 0x50, 0x17, 0xFE, 0x12, 0x06, 0x35, 0x84, 0x81, 0xFB);
    ASSERT_SIMD_EQ(mask_bits_lo(val, 0),   _mm_set_epi8(0xFF, 0x32, 0x17, 0x32, 0x00, 0x19, 0x32, 0x50, 0x17, 0xFE, 0x12, 0x06, 0x35, 0x84, 0x81, 0xFB));
    ASSERT_SIMD_EQ(mask_bits_lo(val, 1),   _mm_set_epi8(0xFF, 0x32, 0x17, 0x32, 0x00, 0x19, 0x32, 0x50, 0x17, 0xFE, 0x12, 0x06, 0x35, 0x84, 0x81, 0xFA));
    ASSERT_SIMD_EQ(mask_bits_lo(val, 64),  _mm_set_epi8(0xFF, 0x32, 0x17, 0x32, 0x00, 0x19, 0x32, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));
    for (size_t i = 158; i < 256; i++) {
        ASSERT_SIMD_EQ(mask_bits_lo(val, i), _mm_setzero_si128());
    }
}

TEST(Simd128, Sr) {
    __m128i val = _mm_set_epi64x(0b1101001101011100101111110101000011111001001101010001111101000100, 0b1010111010101010010111010101011101011011110101010101110100011001);
    ASSERT_SIMD_EQ(sr_bits(val, 0),   _mm_set_epi64x(0b1101001101011100101111110101000011111001001101010001111101000100, 0b1010111010101010010111010101011101011011110101010101110100011001));
    ASSERT_SIMD_EQ(sr_bits(val, 1),   _mm_set_epi64x(0b0110100110101110010111111010100001111100100110101000111110100010, 0b0101011101010101001011101010101110101101111010101010111010001100));
    ASSERT_SIMD_EQ(sr_bits(val, 2),   _mm_set_epi64x(0b0011010011010111001011111101010000111110010011010100011111010001, 0b0010101110101010100101110101010111010110111101010101011101000110));
    ASSERT_SIMD_EQ(sr_bits(val, 8),   _mm_set_epi64x(0b0000000011010011010111001011111101010000111110010011010100011111, 0b0100010010101110101010100101110101010111010110111101010101011101));
    ASSERT_SIMD_EQ(sr_bits(val, 15),  _mm_set_epi64x(0b0000000000000001101001101011100101111110101000011111001001101010, 0b0011111010001001010111010101010010111010101011101011011110101010));
    ASSERT_SIMD_EQ(sr_bits(val, 16),  _mm_set_epi64x(0b0000000000000000110100110101110010111111010100001111100100110101, 0b0001111101000100101011101010101001011101010101110101101111010101));
    ASSERT_SIMD_EQ(sr_bits(val, 124), _mm_set_epi64x(0b0000000000000000000000000000000000000000000000000000000000000000, 0b0000000000000000000000000000000000000000000000000000000000001101));
    ASSERT_SIMD_EQ(sr_bits(val, 125), _mm_set_epi64x(0b0000000000000000000000000000000000000000000000000000000000000000, 0b0000000000000000000000000000000000000000000000000000000000000110));
    ASSERT_SIMD_EQ(sr_bits(val, 126), _mm_set_epi64x(0b0000000000000000000000000000000000000000000000000000000000000000, 0b0000000000000000000000000000000000000000000000000000000000000011));
    ASSERT_SIMD_EQ(sr_bits(val, 127), _mm_set_epi64x(0b0000000000000000000000000000000000000000000000000000000000000000, 0b0000000000000000000000000000000000000000000000000000000000000001));
    for (size_t i = 128; i < 256; i++) {
        ASSERT_SIMD_EQ(sr_bits(val, i), _mm_setzero_si128());
    }
}

TEST(Simd128, Sl) {
    __m128i val = _mm_set_epi64x(0b1101001101011100101111110101000011111001001101010001111101000100, 0b1010111010101010010111010101011101011011110101010101110100011001);
    ASSERT_SIMD_EQ(sl_bits(val, 0),   _mm_set_epi64x(0b1101001101011100101111110101000011111001001101010001111101000100, 0b1010111010101010010111010101011101011011110101010101110100011001));
    ASSERT_SIMD_EQ(sl_bits(val, 1),   _mm_set_epi64x(0b1010011010111001011111101010000111110010011010100011111010001001, 0b0101110101010100101110101010111010110111101010101011101000110010));
    ASSERT_SIMD_EQ(sl_bits(val, 2),   _mm_set_epi64x(0b0100110101110010111111010100001111100100110101000111110100010010, 0b1011101010101001011101010101110101101111010101010111010001100100));
    ASSERT_SIMD_EQ(sl_bits(val, 8),   _mm_set_epi64x(0b0101110010111111010100001111100100110101000111110100010010101110, 0b1010101001011101010101110101101111010101010111010001100100000000));
    ASSERT_SIMD_EQ(sl_bits(val, 15),  _mm_set_epi64x(0b0101111110101000011111001001101010001111101000100101011101010101, 0b0010111010101011101011011110101010101110100011001000000000000000));
    ASSERT_SIMD_EQ(sl_bits(val, 16),  _mm_set_epi64x(0b1011111101010000111110010011010100011111010001001010111010101010, 0b0101110101010111010110111101010101011101000110010000000000000000));
    ASSERT_SIMD_EQ(sl_bits(val, 124), _mm_set_epi64x(0b1001000000000000000000000000000000000000000000000000000000000000, 0b0000000000000000000000000000000000000000000000000000000000000000));
    ASSERT_SIMD_EQ(sl_bits(val, 125), _mm_set_epi64x(0b0010000000000000000000000000000000000000000000000000000000000000, 0b0000000000000000000000000000000000000000000000000000000000000000));
    ASSERT_SIMD_EQ(sl_bits(val, 126), _mm_set_epi64x(0b0100000000000000000000000000000000000000000000000000000000000000, 0b0000000000000000000000000000000000000000000000000000000000000000));
    ASSERT_SIMD_EQ(sl_bits(val, 127), _mm_set_epi64x(0b1000000000000000000000000000000000000000000000000000000000000000, 0b0000000000000000000000000000000000000000000000000000000000000000));
    for (size_t i = 128; i < 256; i++) {
        ASSERT_SIMD_EQ(sl_bits(val, i), _mm_setzero_si128());
    }
}

