#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <util/bits.hpp>

template <size_t tau, size_t N, size_t divisions = 2> 
class wt_twophase_lut {
private:
    // Packed words are packed like this in a packed list.
    // <----------    N..1
    // --> --> -->  tau..1
    // The LUT's job is essentially to rearrange the bits as required to 
    // create the bit vector and the left and right bit vectors

    constexpr static size_t REAL_N = N / divisions;
    static_assert(N % divisions == 0);
    constexpr static size_t B = REAL_N * tau;
    constexpr static size_t SIZE = 1ULL << B;

    // The N bits to extract from a vector of tau-bit values
    // size_t must fit tau*N
    // <---<---<---   N..1
    // <----------- tau..1
    size_t m_bits[SIZE];
    static_assert(sizeof(decltype(m_bits[0])) * 8 >= N * tau);

    // The packed tau-bit values, rearranged to pack values that go into one child together
    // . . --> --> --> --> --> --> --> -->  1
    // . .|--> --> --> --> --> --> --> -->  ..
    // . . --> --> -->|--> --> --> --> -->  tau
    //       right --^  ^-- left
    size_t m_packed[SIZE][tau];
    static_assert(sizeof(decltype(m_packed[0][0])) * 8 >= N * tau);

    // The number of children that go in the *left* child.
    // The number of children in the right child are calculated by REAL_N - ...
    // Used to separate the packed_values from m_packed
    uint8_t m_bitrank[SIZE][tau][REAL_N];
    static_assert(1ULL << (sizeof(decltype(m_bitrank[0][0][0])) * 8) >= (1ULL << REAL_N));

    inline constexpr wt_twophase_lut() : m_bits(), m_packed(), m_bitrank() {
        // 1. Build bits LUT
        for (size_t i = 0; i < SIZE; i++) {  
            size_t value = 0;
            for (size_t j = 0; j < tau; j++) {
                for (size_t c = 0; c < REAL_N; c++) {
                    // <----------  ==> <-- <-- <--- N
                    // --> --> -->      <----------- tau
                    size_t current_value = c * tau;
                    size_t current_bit = tau - 1 - j;
                    auto b = i & (1ULL << (current_value + current_bit));
                    if (b) {
                        value |= 1ULL << (j * REAL_N + c);
                    }
                }
            }
            m_bits[i] = value;
        }

        // 2.Build packed words
        for (size_t j = 0; j < tau; j++) {
            for (size_t i = 0; i < SIZE; i++) {
                size_t value_left = 0;
                size_t value_right = 0;

                size_t c_left = 0;
                size_t c_right = 0;

                for (size_t c = 0; c < REAL_N; c++) {
                    auto packed_value = i >> (c * tau);
                    packed_value &= mask(tau);
                    auto current_bit = tau - j - 1;
                    if (!(packed_value & (1ULL << current_bit))) {
                        value_left |= packed_value << (c_left * tau);
                        c_left++;
                    } else {
                        value_right |= packed_value << (c_right * tau);
                        c_right++;
                    }
                    m_packed[i][j] = value_right << (c_left * tau) | value_left;
                    m_bitrank[i][j][c] = c_left;
                }
            }
        }
    }

    constexpr static wt_twophase_lut singleton_instance = wt_twophase_lut();

public:
    wt_twophase_lut(const wt_twophase_lut&) = delete;
    wt_twophase_lut(const wt_twophase_lut&&) = delete;
    wt_twophase_lut& operator=(const wt_twophase_lut&) = delete;
    wt_twophase_lut& operator=(const wt_twophase_lut&&) = delete;

    // Hopefully this does the trick --v
    __attribute__((optimize("unroll-loops")))
    inline constexpr size_t get_bits(size_t v, size_t alpha) const {
        size_t value = 0;
        for (size_t i = 0; i < divisions; i++) {
            auto current = (m_bits[v & mask(B)] >> (alpha * REAL_N)) & mask(REAL_N);
            value |= current << (REAL_N * i);
            v >>= B;
        }
        return value;
    }

    __attribute__((optimize("unroll-loops")))
    inline constexpr size_t get_packed_left(size_t v, size_t alpha) const {
        size_t value = 0;
        size_t count = 0;
        for (size_t i = 0; i < divisions; i++) {
            auto current_count = m_bitrank[v & mask(B)][alpha][REAL_N - 1];
            auto packed_value = m_packed[v & mask(B)][alpha] & mask(current_count * tau);
            value |= packed_value << (count * tau);
            count += current_count;
            v >>= B;
        }
        return value;
    }

    __attribute__((optimize("unroll-loops")))
    inline constexpr size_t get_packed_right(size_t v, size_t alpha) const {
        size_t value = 0;
        size_t count = 0;
        for (size_t i = 0; i < divisions; i++) {
            auto current_count = m_bitrank[v & mask(B)][alpha][REAL_N - 1];
            auto packed_value = m_packed[v & mask(B)][alpha] >> current_count * tau;
            value |= packed_value << (count * tau);
            count += REAL_N - current_count;
            v >>= B;
        }
        return value;
    }

    __attribute__((optimize("unroll-loops")))
    inline constexpr size_t get_packed_left_count(size_t v, size_t alpha, size_t count = N) const {
        size_t value = 0;
        for (;;) {
            size_t real_count = std::min(count, REAL_N) - 1;
            value += m_bitrank[v & mask(B)][alpha][real_count];
            if (REAL_N >= count) {
                return value;
            }
            v >>= B;
            count -= REAL_N;
        }
    }

    inline constexpr size_t get_packed_right_count(size_t i, size_t alpha, size_t count = N) const {
        return count - get_packed_left_count(i, alpha, count);
    }

    inline constexpr static const wt_twophase_lut& get() {
        // the static_assert forces the compiler to
        // build the singleton_instance during compile time
        static_assert(singleton_instance.get_bits(0, 0) == 0);
        static_assert(singleton_instance.get_packed_left(0, 0) == 0);
        static_assert(singleton_instance.get_packed_right(0, 0) == 0);
        static_assert(singleton_instance.get_packed_left_count(0, 0, N) == N);
        static_assert(singleton_instance.get_packed_right_count(0, 0) == 0);

        return singleton_instance;
    }
};