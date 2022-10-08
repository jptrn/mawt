#pragma once

#include <cstddef>

// Primitive Functions for interacting with byte-packed words, most of it taken from Kaneta 2018

template <typename packed_value_type, size_t tau>
constexpr inline packed_value_type get_l() {
    packed_value_type value = 0;
    for (size_t i = 0; i < sizeof(packed_value_type) * 8; i += tau) {
        value |= 1ULL << i;
    }
    return value;
}

template <typename packed_value_type, size_t tau>
constexpr inline packed_value_type get_h() {
    return get_l<packed_value_type, tau>() << (tau - 1);
}


template <typename packed_value_type, size_t tau>
constexpr inline packed_value_type check1(size_t v, size_t alpha) {
    return (v >> alpha) & get_l<packed_value_type, tau>();
}

template <typename packed_value_type, size_t tau>
constexpr inline packed_value_type check0(size_t v, size_t alpha) {
    return check1<packed_value_type, tau>(v, alpha) ^ get_l<packed_value_type, tau>();
}

template <typename packed_value_type, size_t tau>
constexpr inline packed_value_type fill(size_t v) {
    auto h = get_h<packed_value_type, tau>();
    auto bit0_list = v & get_l<packed_value_type, tau>();
    return (h - bit0_list) ^ h;
}