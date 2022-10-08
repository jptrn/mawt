#pragma once

#include <cstddef>
#include <cstdint>

#include <immintrin.h>

// Extension of the pshufb shuffle mask LUT for 128 bits
// See packed_list_split_pshufb.hpp for details

template <size_t N>
class encoded_compress_shufmask_128_lut {
private:
    static_assert(sizeof(__m128i) >= N);

    // This is about as big as it gets, it probably is infeasible to use this approach
    // for 256/512 bit registers - pshufb exists for 512 bits, but I'll probably
    // need to construct a shuffle mask from several smaller ones
    // plus there's so many other interesting structures
    static constexpr size_t SIZE = 1ULL << N;
    uint8_t m_data[SIZE][sizeof(__m128i)];

    inline constexpr encoded_compress_shufmask_128_lut() : m_data() {
        for (size_t i = 0; i < SIZE; i++) {
            size_t c = 0;
            for (size_t j = 0; j < N; j++) {
                if (i & (1ULL << j)) {
                    m_data[i][c++] = j;
                }   
            }
            for (size_t j = c; j < 16; j++) {
                m_data[i][j] = 0xFF;
            }
        }
    }

    constexpr static encoded_compress_shufmask_128_lut singleton_instance = encoded_compress_shufmask_128_lut();

public:
    encoded_compress_shufmask_128_lut(const encoded_compress_shufmask_128_lut&) = delete;
    encoded_compress_shufmask_128_lut(const encoded_compress_shufmask_128_lut&&) = delete;
    encoded_compress_shufmask_128_lut& operator=(const encoded_compress_shufmask_128_lut&) = delete;
    encoded_compress_shufmask_128_lut& operator=(const encoded_compress_shufmask_128_lut&&) = delete;

    inline constexpr __m128i get_shuffle_mask(size_t bits) const {
        __m128i* ptr = (__m128i*)m_data[bits];
        return *ptr;
    }

    inline constexpr static const encoded_compress_shufmask_128_lut& get() {
        static_assert(N == 16, "pshufb impl not supported for N != 16!");
        static_assert(singleton_instance.m_data[0][0] == 0xFF);

        return singleton_instance;
    }
};