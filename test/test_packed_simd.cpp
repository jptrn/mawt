#include <gtest/gtest.h>

#include <wt/packed/packed_data.hpp>
#include <wt/packed/packed_shuffle_provider.hpp>

#include <cstddef>
#include <cstdint>

#include <immintrin.h>

#include "util/simd_util.hpp"

TEST(PextPshufb128, CountZeros8) {
    ASSERT_EQ(count_zeros_8(_mm_setzero_si128(), 0, 0), 0);
    ASSERT_EQ(count_zeros_8(_mm_setzero_si128(), 0, 1), 1);
    ASSERT_EQ(count_zeros_8(_mm_setzero_si128(), 0, 2), 2);
    ASSERT_EQ(count_zeros_8(_mm_setzero_si128(), 0, 15), 15);
    ASSERT_EQ(count_zeros_8(_mm_setzero_si128(), 0, 16), 16);
    
    __m128i val = _mm_set_epi8(
        0b0100'0000, // ^ N
        0b1100'0001, // |
        0b1100'0001, // |
        0b1100'0000, // |
        0b1100'0000, // |
        0b1100'0000, // |
        0b1100'0001, // |
        0b1100'0000, // |
        0b1100'0000, // |
        0b1100'0000, // |
        0b1100'0000, // |
        0b0100'0001, // |
        0b0100'0000, // |
        0b1100'0000, // |
        0b0100'0000, // 
        0b1100'0000  // 1 -----> tau
    );
    ASSERT_EQ(count_zeros_8(val, 0, 1), 0);
    ASSERT_EQ(count_zeros_8(val, 0, 2), 1);
    ASSERT_EQ(count_zeros_8(val, 0, 3), 1);
    ASSERT_EQ(count_zeros_8(val, 0, 4), 2);
    ASSERT_EQ(count_zeros_8(val, 0, 5), 3);
    ASSERT_EQ(count_zeros_8(val, 0, 15), 3);
    ASSERT_EQ(count_zeros_8(val, 0, 16), 4);

    ASSERT_EQ(count_zeros_8(val, 1, 1), 0);
    ASSERT_EQ(count_zeros_8(val, 1, 16), 0);

    ASSERT_EQ(count_zeros_8(val, 6, 16), 16);

    ASSERT_EQ(count_zeros_8(val, 7, 1), 1);
    ASSERT_EQ(count_zeros_8(val, 7, 16), 12);
}

TEST(PextPshufb256, CountZeros8) {
    ASSERT_EQ(count_zeros_8(_mm256_setzero_si256(), 0, 0), 0);
    ASSERT_EQ(count_zeros_8(_mm256_setzero_si256(), 0, 1), 1);
    ASSERT_EQ(count_zeros_8(_mm256_setzero_si256(), 0, 2), 2);
    ASSERT_EQ(count_zeros_8(_mm256_setzero_si256(), 0, 31), 31);
    ASSERT_EQ(count_zeros_8(_mm256_setzero_si256(), 0, 32), 32);
    
    __m256i val = _mm256_set_epi8(
        0b1010'1001,
        0b0010'1000, 
        0b0010'1001, 
        0b1010'1001,
        0b1010'1000, 
        0b1010'1000, 
        0b0010'1001, 
        0b0010'1001,
        0b1010'1001, 
        0b1010'1000, 
        0b0010'1000,
        0b0010'1001,
        0b1010'1001,
        0b1010'1001, 
        0b0010'1000,
        0b1010'1001,
        0b1010'1001,
        0b0010'1000, 
        0b0010'1001,
        0b1010'1001,
        0b1010'1000,
        0b1010'1000, 
        0b0010'1001,
        0b0010'1001, 
        0b1010'1001,
        0b1010'1000, 
        0b0010'1000,
        0b0010'1001,
        0b1010'1001,
        0b1010'1001, 
        0b0010'1000,
        0b1010'1001
    );
    ASSERT_EQ(count_zeros_8(val, 0, 1), 0);
    ASSERT_EQ(count_zeros_8(val, 0, 2), 1);
    ASSERT_EQ(count_zeros_8(val, 0, 3), 1);
    ASSERT_EQ(count_zeros_8(val, 0, 4), 1);
    ASSERT_EQ(count_zeros_8(val, 0, 5), 2);
    ASSERT_EQ(count_zeros_8(val, 0, 15), 7);
    ASSERT_EQ(count_zeros_8(val, 0, 32), 14);

    ASSERT_EQ(count_zeros_8(val, 4, 0), 0);
    ASSERT_EQ(count_zeros_8(val, 4, 8), 0);
    ASSERT_EQ(count_zeros_8(val, 4, 32), 0);

    ASSERT_EQ(count_zeros_8(val, 5, 0), 0);
    ASSERT_EQ(count_zeros_8(val, 5, 1), 1);
    ASSERT_EQ(count_zeros_8(val, 5, 32), 32);

    ASSERT_EQ(count_zeros_8(val, 7, 32), 12);
}

TEST(PextPshufb128, PackBits128) {
    __m128i val = _mm_set_epi8(
        0b1010'0001, 
        0b0010'0000, 
        0b0010'0001, 
        0b1010'0001, 
        0b1010'0000, 
        0b1010'0000, 
        0b0010'0001, 
        0b0010'0001, 
        0b1010'0001, 
        0b1010'0000, 
        0b0010'0000, 
        0b0010'0001, 
        0b1010'0001, 
        0b1010'0001, 
        0b0010'0000, 
        0b1010'0001
    );
    ASSERT_EQ(extract_8(val, 0, 0),  0b0000'0000'0000'0000);
    ASSERT_EQ(extract_8(val, 0, 1),  0b0000'0000'0000'0001);
    ASSERT_EQ(extract_8(val, 0, 4),  0b0000'0000'0000'1101);
    ASSERT_EQ(extract_8(val, 0, 15), 0b0001'1100'1100'1101);
    ASSERT_EQ(extract_8(val, 0, 16), 0b1001'1100'1100'1101);

    ASSERT_EQ(extract_8(val, 1, 0),  0b0000'0000'0000'0000);
    ASSERT_EQ(extract_8(val, 1, 1),  0b0000'0000'0000'0000);
    ASSERT_EQ(extract_8(val, 1, 16), 0b0000'0000'0000'0000);

    ASSERT_EQ(extract_8(val, 2, 0),  0b0000'0000'0000'0000);
    ASSERT_EQ(extract_8(val, 2, 1),  0b0000'0000'0000'0001);
    ASSERT_EQ(extract_8(val, 2, 16), 0b1111'1111'1111'1111);

    ASSERT_EQ(extract_8(val, 7, 16), 0b1011'0011'1001'1101);
}


TEST(PextPshufb256, PackBits256) {
    __m256i val = _mm256_set_epi8(
        0b1010'0001,
        0b0010'0000, 
        0b0010'0001, 
        0b1010'0001,
        0b1010'0000, 
        0b1010'0000, 
        0b0010'0001, 
        0b0010'0001,
        0b1010'0001, 
        0b1010'0000, 
        0b0010'0000,
        0b0010'0001,
        0b1010'0001,
        0b1010'0001, 
        0b0010'0000,
        0b1010'0001,
        0b1010'0001,
        0b0010'0000, 
        0b0010'0001,
        0b1010'0001,
        0b1010'0000,
        0b1010'0000, 
        0b0010'0001,
        0b0010'0001, 
        0b1010'0001,
        0b1010'0000, 
        0b0010'0000,
        0b0010'0001,
        0b1010'0001,
        0b1010'0001, 
        0b0010'0000,
        0b1010'0000
    );
    ASSERT_EQ(extract_8(val, 0, 0),  0b0000'0000'0000'0000'0000'0000'0000'0000);
    ASSERT_EQ(extract_8(val, 0, 1),  0b0000'0000'0000'0000'0000'0000'0000'0001);
    ASSERT_EQ(extract_8(val, 0, 4),  0b0000'0000'0000'0000'0000'0000'0000'1101);
    ASSERT_EQ(extract_8(val, 0, 15), 0b0000'0000'0000'0000'0001'1100'1100'1101);
    ASSERT_EQ(extract_8(val, 0, 32), 0b1001'1100'1100'1101'1001'1100'1100'1101);

    ASSERT_EQ(extract_8(val, 1, 0),  0b0000'0000'0000'0000'0000'0000'0000'0000);
    ASSERT_EQ(extract_8(val, 1, 1),  0b0000'0000'0000'0000'0000'0000'0000'0000);
    ASSERT_EQ(extract_8(val, 1, 32), 0b0000'0000'0000'0000'0000'0000'0000'0000);

    ASSERT_EQ(extract_8(val, 2, 0),  0b0000'0000'0000'0000'0000'0000'0000'0000);
    ASSERT_EQ(extract_8(val, 2, 1),  0b0000'0000'0000'0000'0000'0000'0000'0001);
    ASSERT_EQ(extract_8(val, 2, 32), 0b1111'1111'1111'1111'1111'1111'1111'1111);

    ASSERT_EQ(extract_8(val, 7, 32), 0b1011'0011'1001'1101'1011'0011'1001'1100);
}
