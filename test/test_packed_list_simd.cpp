#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

#include <util/bits.hpp>
#include <vector/packed/packed_list.hpp>
#include <vector/packed/packed_list_simd.hpp>
#include <vector/packed/packed_list_writer.hpp>
#include <vector/packed/packed_util_simd.hpp>

#include "util/simd_util.hpp"

#include <immintrin.h>

TEST(PackedListSimd, Basic128) {
    packed_list<8, __m128i> p(516);
    packed_list_writer<8, __m128i> pw(p);
    ASSERT_EQ(516, p.size());

    std::vector<size_t> expected(p.size());
    for (size_t i = 0; i < p.size(); i++) {
        expected[i] = ((i % 27) + (i % 4)) & mask(8);
        pw.write(expected[i]);
    }

    // Aligned reads
    for (size_t i = 0; i < p.size(); i += 16) {
        __m128i e = _mm_setzero_si128();
        for (size_t r = 1; r <= 16; r++) {
            size_t read = r - 1;
            if (i + read < p.size()) {
                __m128i next_value = _mm_set_epi64x(0, expected[i + read]);
                e = _mm_or_si128(e, sl_8(next_value, read));
                ASSERT_SIMD_EQ(p.pack_aligned(i, r), e);
            }
        }
    }   

    // Unaligned reads
    for (size_t i = 0; i < p.size(); i++) {
        __m128i e = _mm_setzero_si128();
        size_t cur_word_pos = i % 16;
        for (size_t r = 1; cur_word_pos + r <= 16; r++) {
            size_t read = r - 1;
            if (i + read < p.size()) {
                __m128i next_value = _mm_set_epi64x(0, expected[i + read]);
                e = _mm_or_si128(e, sl_8(next_value, read));
                ASSERT_SIMD_EQ(p.pack(i, r), e);
            }
        }
    }   
}

TEST(PackedListSimd, Basic256) {
    packed_list<8, __m256i> p(516);
    packed_list_writer<8, __m256i> pw(p);
    ASSERT_EQ(516, p.size());

    std::vector<size_t> expected(p.size());
    for (size_t i = 0; i < p.size(); i++) {
        expected[i] = ((i % 27) + (i % 4)) & mask(8);
        pw.write(expected[i]);
    }

    // Aligned reads
    for (size_t i = 0; i < p.size(); i += 32) {
        __m256i e = _mm256_setzero_si256();
        for (size_t r = 1; r <= 32; r++) {
            size_t read = r - 1;
            if (i + read < p.size()) {
                __m256i next_value = _mm256_set_epi64x(0, 0, 0, expected[i + read]);
                e = _mm256_or_si256(e, sl_8(next_value, read));
                ASSERT_SIMD_EQ(p.pack_aligned(i, r), e);
            }
        }
    }   

    // Unaligned reads
    for (size_t i = 0; i < p.size(); i++) {
        __m256i e = _mm256_setzero_si256();
        size_t cur_word_pos = i % 32;
        for (size_t r = 1; cur_word_pos + r <= 32; r++) {
            size_t read = r - 1;
            if (i + read < p.size()) {
                __m256i next_value = _mm256_set_epi64x(0, 0, 0, expected[i + read]);
                e = _mm256_or_si256(e, sl_8(next_value, read));
                ASSERT_SIMD_EQ(p.pack(i, r), e);
            }
        }
    }   
}

TEST(PackedListSimd, Basic512) {
    packed_list<8, __m512i> p(516);
    packed_list_writer<8, __m512i> pw(p);
    ASSERT_EQ(516, p.size());

    std::vector<size_t> expected(p.size());
    for (size_t i = 0; i < p.size(); i++) {
        expected[i] = ((i % 27) + (i % 4)) & mask(8);
        pw.write(expected[i]);
    }

    // Aligned reads
    for (size_t i = 0; i < p.size(); i += 64) {
        __m512i e = _mm512_setzero_si512();
        for (size_t r = 1; r <= 64; r++) {
            size_t read = r - 1;
            if (i + read < p.size()) {
                __m512i next_value = _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, expected[i + read]);
                e = _mm512_or_si512(e, sl_8(next_value, read));
                ASSERT_SIMD_EQ(p.pack_aligned(i, r), e);
            }
        }
    }   

    // Unaligned reads
    for (size_t i = 0; i < p.size(); i++) {
        __m512i e = _mm512_setzero_si512();
        size_t cur_word_pos = i % 64;
        for (size_t r = 1; cur_word_pos + r <= 64; r++) {
            size_t read = r - 1;
            if (i + read < p.size()) {
                __m512i next_value = _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, expected[i + read]);
                e = _mm512_or_si512(e, sl_8(next_value, read));
                ASSERT_SIMD_EQ(p.pack(i, r), e);
            }
        }
    }   
}

TEST(PackedListSimd, WriteMultiple128) {
    packed_list<8, __m128i> p(516);
    packed_list_writer<8, __m128i> pw(p);
    ASSERT_EQ(516, p.size());

    // Write 'n' values at a time, where n is taken from a random permutation of [0, 16]
    std::vector<size_t> write_counts(17);
    std::iota(write_counts.begin(), write_counts.end(), 0);
    std::random_shuffle(write_counts.begin(), write_counts.end());
    size_t shuffle_read_i = 0;

    std::vector<size_t> expected(p.size());
    for (size_t i = 0; i < p.size();) {
        size_t values_to_write = std::min(p.size() - i, write_counts[shuffle_read_i++]);
        shuffle_read_i %= write_counts.size();
        __m128i e = _mm_setzero_si128();
        for (size_t v = 0; v < values_to_write; v++) {
            expected[i + v] = (331 + (i % 31) + (i % 7)) & mask(8);
            __m128i next_value = _mm_set_epi64x(0, expected[i + v]);
            e = _mm_or_si128(e, sl_8(next_value, v));
        }
        pw.write(values_to_write, e);
        i += values_to_write;
    }

    for (size_t i = 0; i < p.size(); i++) {
        ASSERT_SIMD_EQ(p.pack(i, 1), _mm_set_epi64x(0, expected[i]));
    }
}

TEST(PackedListSimd, WriteMultiple256) {
    packed_list<8, __m256i> p(516);
    packed_list_writer<8, __m256i> pw(p);
    ASSERT_EQ(516, p.size());

    // Write 'n' values at a time, where n is taken from a random permutation of [0, 16]
    std::vector<size_t> write_counts(33);
    std::iota(write_counts.begin(), write_counts.end(), 0);
    std::random_shuffle(write_counts.begin(), write_counts.end());
    size_t shuffle_read_i = 0;

    std::vector<size_t> expected(p.size());
    for (size_t i = 0; i < p.size();) {
        size_t values_to_write = std::min(p.size() - i, write_counts[shuffle_read_i++]);
        shuffle_read_i %= write_counts.size();
        __m256i e = _mm256_setzero_si256();
        for (size_t v = 0; v < values_to_write; v++) {
            expected[i + v] = (331 + (i % 31) + (i % 7)) & mask(8);
            __m256i next_value = _mm256_set_epi64x(0, 0, 0, expected[i + v]);
            e = _mm256_or_si256(e, sl_8(next_value, v));
        }
        pw.write(values_to_write, e);
        i += values_to_write;
    }

    for (size_t i = 0; i < p.size(); i++) {
        ASSERT_SIMD_EQ(p.pack(i, 1), _mm256_set_epi64x(0, 0, 0, expected[i]));
    }
}

TEST(PackedListSimd, WriteMultiple512) {
    packed_list<8, __m512i> p(516);
    packed_list_writer<8, __m512i> pw(p);
    ASSERT_EQ(516, p.size());

    // Write 'n' values at a time, where n is taken from a random permutation of [0, 16]
    std::vector<size_t> write_counts(65);
    std::iota(write_counts.begin(), write_counts.end(), 0);
    std::random_shuffle(write_counts.begin(), write_counts.end());
    size_t shuffle_read_i = 0;

    std::vector<size_t> expected(p.size());
    for (size_t i = 0; i < p.size();) {
        size_t values_to_write = std::min(p.size() - i, write_counts[shuffle_read_i++]);
        shuffle_read_i %= write_counts.size();
        __m512i e = _mm512_setzero_si512();
        for (size_t v = 0; v < values_to_write; v++) {
            expected[i + v] = (331 + (i % 31) + (i % 7)) & mask(8);
            __m512i next_value = _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, expected[i + v]);
            e = _mm512_or_si512(e, sl_8(next_value, v));
        }
        pw.write(values_to_write, e);
        i += values_to_write;
    }

    for (size_t i = 0; i < p.size(); i++) {
        ASSERT_SIMD_EQ(p.pack(i, 1), _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, expected[i]));
    }
}