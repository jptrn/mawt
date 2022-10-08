#pragma once

#include <wt/encoded/encoded_value_compress_lut.hpp>
#include <wt/packed/packed_bit_util.hpp>

#include <cstddef>
#include <tuple>

#include <immintrin.h>

// ok, so, how to use broadword programming to find out which of the tau-bit (tau <= 8) values appear in the next level
// we basically wanna know if the code_length is > alpha for all values at once
// this is the best scheme I could come up with
// 
// we know that each length only takes log2(tau) bits in the word, so we should be able to add 
// tau - alpha to each value using broadword programming. if the bit @ log2(tau) is set, that means the value should appear in the next level

// example tau = 4
// lengths        0000 0001 0010 0011 0100
// alpha = 0  011 0011 0100 0101 0110 0111
// alpha = 1  010 0010 0011 0100 0101 0110
// alpha = 2  001 0001 0010 0011 0100 0101
// alpha = 3  000 0000 0001 0010 0011 0100

template <typename packed_enc_value_type, size_t tau>
inline size_t get_compress_mask_bit_extract(packed_enc_value_type l, size_t alpha, size_t count) {
    size_t length_bits = log2_ceil(tau);
    packed_enc_value_type max_mask = get_l<packed_enc_value_type, tau>() << length_bits;
    // Yep, we really need a LUT for this. there's probably some fancy broadword implementation for this, but thinking is hard
    const auto& compress_mask_lut = encoded_value_compress_lut<packed_enc_value_type, tau>::get();
    l += compress_mask_lut.get_add_mask(alpha);
    l &= max_mask;
    return _pext_u64(l, max_mask) & mask(count);
}

template <typename packed_enc_value_type, size_t tau>
static inline std::tuple<packed_enc_value_type, packed_enc_value_type, size_t> compress_values_bit_extract(packed_enc_value_type v, packed_enc_value_type l, size_t alpha, size_t count) {
    size_t bits = get_compress_mask_bit_extract<packed_enc_value_type, tau>(l, alpha, count);
    size_t new_count = __builtin_popcountll(bits);
    packed_enc_value_type mask = _pdep_u64(bits, get_l<packed_enc_value_type, tau>());
    mask = fill<packed_enc_value_type, tau>(mask);
    return { _pext_u64(v, mask), _pext_u64(l, mask), new_count };
}
