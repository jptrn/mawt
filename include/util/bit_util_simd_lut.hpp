#pragma once

#include <cstdint>
#include <cstddef>

/*
 * Similar to bit_util_simd_lut, but for bits
 * 
 * `simd_type` indicates which SIMD register type to produce shuffle masks for
 */
template <typename simd_type> 
class bit_util_simd_lut {
private:
    static constexpr size_t BYTE_SIZE = sizeof(simd_type) * 8;
    static constexpr size_t BIT_SIZE = sizeof(simd_type) * 8;

    uint8_t m_mask[BYTE_SIZE][BIT_SIZE] __attribute__((aligned (sizeof(simd_type))));

    inline constexpr bit_util_simd_lut() : 
        m_mask() {

        // Mask
        for (size_t i = 0; i < BIT_SIZE; i++) {
            for (size_t j = 0; j < i; j += 8) {
                m_mask[i][j / 8] = j + 8 <= i ? 0xFF : ((1ULL << (i & 7)) - 1);
            }
        }
    }

    constexpr static bit_util_simd_lut singleton_instance = bit_util_simd_lut();

public:
    bit_util_simd_lut(const bit_util_simd_lut&) = delete;
    bit_util_simd_lut(const bit_util_simd_lut&&) = delete;
    bit_util_simd_lut& operator=(const bit_util_simd_lut&) = delete;
    bit_util_simd_lut& operator=(const bit_util_simd_lut&&) = delete;

    inline constexpr simd_type mask(size_t n) const {
        auto* p_mask = (simd_type*)m_mask[n];
        return *p_mask;
    }


    inline constexpr static const bit_util_simd_lut& get() {
        static_assert(singleton_instance.m_mask[0][0] == 0);
        return singleton_instance;
    }
};
