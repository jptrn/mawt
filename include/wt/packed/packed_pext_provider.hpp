#pragma once

#include <util/bits.hpp>
#include <wt/packed/packed_bit_util.hpp>
#include <wt/packed/packed_data.hpp>

#include <cstddef>

#include <immintrin.h>

template <typename packed_value_type, size_t tau>
inline size_t extract_pext(packed_value_type v, size_t alpha, size_t count) {
    alpha = tau - alpha - 1;
    size_t l = get_l<packed_value_type, tau>();
    return _pext_u64(v & mask(tau * count), l << alpha);
}

template <typename packed_value_type, size_t tau>
inline packed_data<packed_value_type> get_packed_data_pext_pext(packed_value_type v, size_t alpha, size_t count) {
    // Pack
    size_t bits = extract_pext<packed_value_type, tau>(v, alpha, count);
    alpha = tau - alpha - 1;

    // Split
    size_t right_values = check1<size_t, tau>(v, alpha);        // if a value is >= count, it should always be 0
    size_t right_count = __builtin_popcountll(right_values);  // idk why _popcnt64 doesn't work here

    size_t left_values = (right_values ^ get_l<size_t, tau>()) & mask(tau * count); // mask out values >= count here
    size_t left_count = count - right_count;

    size_t right_mask = fill<size_t, tau>(right_values);
    size_t left_mask = fill<size_t, tau>(left_values);

    packed_value_type right = _pext_u64(v, right_mask);
    packed_value_type left = _pext_u64(v, left_mask);
 
    return packed_data<packed_value_type>(left_count, right_count, left, right, bits);
}

template <typename packed_value_type, size_t tau> 
inline size_t count_zeros_tau(packed_value_type v, size_t alpha, size_t count) {
    auto c = check0<packed_value_type, tau>(v, tau - alpha - 1);      
    return __builtin_popcountll(c & mask(tau * count));  
}
