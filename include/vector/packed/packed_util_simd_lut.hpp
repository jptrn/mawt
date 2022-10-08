#pragma once

#include <cstdint>
#include <cstddef>

/*
 * This class provides static lookup tables for shuffle masks for certain SIMD
 * operations. See `packed_util_simd.hpp` for details
 *
 * `simd_type` indicates which SIMD register type to produce shuffle masks for
 * `value_type` indicates the width of the individual value, i.e. bytes, shorts, etc.
 */
template <typename simd_type, typename value_type = uint8_t> 
class packed_util_simd_lut {
private:
    static constexpr size_t SIZE = sizeof(simd_type) / sizeof(value_type) ;

    value_type m_mask[SIZE][SIZE] __attribute__((aligned (sizeof(simd_type))));
    value_type m_sl[SIZE][SIZE] __attribute__((aligned (sizeof(simd_type))));
    value_type m_sr[SIZE][SIZE] __attribute__((aligned (sizeof(simd_type))));

    inline constexpr packed_util_simd_lut() : 
        m_mask(), 
        m_sl(), 
        m_sr() {

        for (size_t i = 0; i < SIZE; i++) {
            for (size_t j = 0; j < SIZE; j++) {
                m_mask[i][j] = j < i ? 0xFF : 0;
                m_sl[i][j] = j >= i ? j - i : 0xFF;
                m_sr[i][j] = j + i < SIZE ? j + i : 0xFF;
            }
        };
    }

    constexpr static packed_util_simd_lut singleton_instance = packed_util_simd_lut();

public:
    packed_util_simd_lut(const packed_util_simd_lut&) = delete;
    packed_util_simd_lut(const packed_util_simd_lut&&) = delete;
    packed_util_simd_lut& operator=(const packed_util_simd_lut&) = delete;
    packed_util_simd_lut& operator=(const packed_util_simd_lut&&) = delete;

    inline constexpr simd_type mask(size_t n) const {
        auto* p_mask = (simd_type*)m_mask[n];
        return *p_mask;
    }

    inline constexpr simd_type sl(size_t n) const {    
        auto* p_mask = (simd_type*)m_sl[n];    
        return *p_mask;
    }

    inline constexpr simd_type sr(size_t n) const {    
        auto* p_mask = (simd_type*)m_sr[n];    
        return *p_mask;
    }

    inline constexpr static const packed_util_simd_lut& get() {
        static_assert(singleton_instance.m_sr[0][0] == 0);
        static_assert(singleton_instance.m_sr[0][1] == 1);
        static_assert(singleton_instance.m_sr[0][2] == 2);
        static_assert(singleton_instance.m_sr[0][sizeof(simd_type) - 1] == sizeof(simd_type) - 1);
        static_assert(singleton_instance.m_sr[1][0] == 1);
        static_assert(singleton_instance.m_sr[1][1] == 2);
        static_assert(singleton_instance.m_sr[1][2] == 3);
        static_assert(singleton_instance.m_sr[1][sizeof(simd_type) - 1] == 0xFF);
        return singleton_instance;
    }
};
