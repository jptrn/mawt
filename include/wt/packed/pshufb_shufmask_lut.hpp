#pragma once

#include <cstddef>
#include <cstdint>

/*
 * This lookup table provides shuffle masks for the pshufb-based implementation
 * of the list splitting operation of Kaneta 2018
 */
template <typename packed_value_type = uint64_t, size_t N = 8> 
class pshufb_shufmask_lut {
private:
    // The general idea is as follows:
    // I use pext to extract a bitmask from a certain word,
    // e.g.   1011 1101 1001 1000
    //         |    |    |    |
    //  0100 <-+----+----+----+
    // This bitmask determines which shuffle mask to generate for pshufb
    //         shuf mask
    //         L <-------> R | L_Count
    // 0000 -> 0   1   2   3 | 4
    // 0001 -> 0   1   2 | 3 | 3
    // 0010 -> 0   1   3 | 2 | 3
    // 0011 -> 0   1 | 2   3 | 2
    // 0100 -> 0   2   3 | 1 | 3
    // 0101 -> 0   2 | 1   3 | 2
    // ...and so on
    // I need to save the shuffle mask -> the count can be implemented with any
    // O(1) popcount implementation, which (could) also be a LUT
    // I need to mind the endianness of the bits here, just to be sure,
    // and the byte endianness for pshufb, since it's reversed from my little
    // implementation
    // alternatively, i can simply use two separate shuffle masks for left and right separated

    static_assert(sizeof(packed_value_type) >= N);

    static constexpr size_t SIZE = 1ULL << N;
    packed_value_type m_data[SIZE];

    inline constexpr pshufb_shufmask_lut() : m_data() {
        for (size_t i = 0; i < SIZE; i++) {
            packed_value_type shuffle_mask_l = 0;    // would be better to use -1ULL and clear out bytes that are actually set
            packed_value_type shuffle_mask_r = 0;
            size_t c_left = 0;
            size_t c_right = 0;
            for (size_t j = 0; j < N; j++) {
                if ((i & (1ULL << j)) == 0) {
                    shuffle_mask_l |= j << (c_left * 8);
                    c_left++;
                } else {
                    shuffle_mask_r |= j << (c_right * 8);
                    c_right++;
                }

            }
            m_data[i] = c_right != 0 ?
                (shuffle_mask_r << (c_left * 8)) | shuffle_mask_l :
                shuffle_mask_l;
        }
    }

    constexpr static pshufb_shufmask_lut singleton_instance = pshufb_shufmask_lut();

public:
    pshufb_shufmask_lut(const pshufb_shufmask_lut&) = delete;
    pshufb_shufmask_lut(const pshufb_shufmask_lut&&) = delete;
    pshufb_shufmask_lut& operator=(const pshufb_shufmask_lut&) = delete;
    pshufb_shufmask_lut& operator=(const pshufb_shufmask_lut&&) = delete;

    inline constexpr packed_value_type get_shuffle_mask(size_t bits) const {
        return m_data[bits];
    }

    inline constexpr static const pshufb_shufmask_lut& get() {

        // the static_assert forces the compiler to
        // build the singleton_instance during compile time
        static_assert(N == 8, "pshufb impl not supported for N != 8!");
        static_assert(singleton_instance.get_shuffle_mask(0b00000000) == 0x0706050403020100);
        static_assert(singleton_instance.get_shuffle_mask(0b00000001) == 0x0007060504030201);
        static_assert(singleton_instance.get_shuffle_mask(0b00000010) == 0x0107060504030200);
        static_assert(singleton_instance.get_shuffle_mask(0b00000011) == 0x0100070605040302);
        static_assert(singleton_instance.get_shuffle_mask(0b11111111) == 0x0706050403020100);

        return singleton_instance;
    }
};