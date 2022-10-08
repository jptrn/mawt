#pragma once

#pragma once

#include <cstddef>
#include <cstdint>

/*
 * This lookup table provides LUTs for bit flipping
 */
template <size_t tau = 8> 
class bits_rev_lut {
private:
    static constexpr size_t SIZE = 1ULL << tau;
    size_t m_data[SIZE];

    inline constexpr bits_rev_lut() : m_data() {
        for (size_t i = 0; i < SIZE; i++) {
            size_t reversed = 0;
            for (size_t j = 0; j < tau; j++) {
                auto bit = i & (1ULL << (tau - j - 1));
                if (bit) {
                    reversed |= 1ULL << j;
                }
            }
            m_data[i] = reversed;
        }
    }

    constexpr static bits_rev_lut singleton_instance = bits_rev_lut();

public:
    bits_rev_lut(const bits_rev_lut&) = delete;
    bits_rev_lut(const bits_rev_lut&&) = delete;
    bits_rev_lut& operator=(const bits_rev_lut&) = delete;
    bits_rev_lut& operator=(const bits_rev_lut&&) = delete;

    inline constexpr size_t reverse(size_t bits) const {
        return m_data[bits];
    }

    inline constexpr static const bits_rev_lut& get() {
        static_assert(singleton_instance.reverse(0) == 0);
        static_assert(singleton_instance.reverse(0b10) == 0b1 << (tau - 2));
        return singleton_instance;
    }
};