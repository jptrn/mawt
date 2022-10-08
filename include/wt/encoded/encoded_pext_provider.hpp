#pragma once

#include <util/bits.hpp>
#include <wt/encoded/packed_enc_data.hpp>
#include <wt/packed/packed_bit_util.hpp>
#include <wt/packed/packed_pext_provider.hpp>

#include <cstddef>

#include <immintrin.h>

#include <util/debug.hpp>

template <typename packed_enc_value_type, size_t tau>
inline packed_enc_data<packed_enc_value_type> split_enc_pext(packed_enc_value_type v, packed_enc_value_type l, size_t alpha, size_t count) {
    alpha = tau - alpha - 1;

    // Split
    size_t right_values = check1<size_t, tau>(v, alpha);        // if a value is >= count, it should always be 0
    size_t right_count = __builtin_popcountll(right_values);  // idk why _popcnt64 doesn't work here

    size_t left_values = (right_values ^ get_l<size_t, tau>()) & mask(tau * count); // mask out values >= count here
    size_t left_count = count - right_count;

    size_t right_mask = fill<size_t, tau>(right_values);
    size_t left_mask = fill<size_t, tau>(left_values);

    packed_enc_value_type right = _pext_u64(v, right_mask);
    packed_enc_value_type left = _pext_u64(v, left_mask);

    packed_enc_value_type right_lengths = _pext_u64(l, right_mask);
    packed_enc_value_type left_lengths = _pext_u64(l, left_mask); 

    return packed_enc_data<packed_enc_value_type>(left_count, right_count, left, right, left_lengths, right_lengths);
}
