#pragma once

#include <util/bits.hpp>

#include <cstddef>
#include <cstdint>

template <typename packed_enc_value_type, size_t N>
class encoded_compress_shufmask_lut {
private:
    static_assert(sizeof(packed_enc_value_type) >= N);

    static constexpr size_t SIZE = 1ULL << N;
    packed_enc_value_type m_data[SIZE];

    inline constexpr encoded_compress_shufmask_lut() : m_data() {
        for (size_t i = 0; i < SIZE; i++) {
            packed_enc_value_type shuffle_mask = 0;
            size_t c = 0;
            for (size_t j = 0; j < N; j++) {
                if (i & (1ULL << j)) {
                    shuffle_mask |= j << (c++ * 8);
                }
            }
            m_data[i] = (-1ULL & ~mask(c * 8)) | shuffle_mask;
        }
    }

    constexpr static encoded_compress_shufmask_lut singleton_instance = encoded_compress_shufmask_lut();

public:
    encoded_compress_shufmask_lut(const encoded_compress_shufmask_lut&) = delete;
    encoded_compress_shufmask_lut(const encoded_compress_shufmask_lut&&) = delete;
    encoded_compress_shufmask_lut& operator=(const encoded_compress_shufmask_lut&) = delete;
    encoded_compress_shufmask_lut& operator=(const encoded_compress_shufmask_lut&&) = delete;

    inline constexpr packed_enc_value_type get_shuffle_mask(size_t bits) const {
        return m_data[bits];
    }

    inline constexpr static const encoded_compress_shufmask_lut& get() {
        static_assert(singleton_instance.get_shuffle_mask(0b00000000 & mask(N)) == (0xFFFFFFFF'FFFFFFFF & mask(N * 8)));
        static_assert(singleton_instance.get_shuffle_mask(0b11111111 & mask(N)) == (0x07060504'03020100 & mask(N * 8)));
        return singleton_instance;
    }
};