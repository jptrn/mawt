#pragma once

#include <util/bits_rev_lut.hpp>
#include <util/debug.hpp>

#include <cstddef>

constexpr size_t log2_ceil(size_t value) {
    return value == 0 ? 0 : (64ULL - __builtin_clzll(value - 1));
}

constexpr size_t mask(size_t bits) {
    return bits >= 64 ? -1ULL : (1ULL << bits) - 1;
}

constexpr size_t divceil(size_t a, size_t b) {
    return (a / b) + (a % b == 0 ? 0 : 1);
} 

size_t bit_reverse(size_t bits, size_t n) {
    size_t reversed = 0;
    for (size_t j = 0; j < n; j++) {
        auto bit = bits & (1ULL << (n - j - 1));
        if (bit) {
            reversed |= 1ULL << j;
        }
    }
    return reversed;
}

// O(1) bit_reverse using a lookup, tau limits the maximum size of the lookup
template <size_t tau>
size_t bit_reverse_max(size_t bits, size_t n = tau) {
    assert(n <= tau && bits < (1ULL << tau));
    const auto& lut = bits_rev_lut<tau>::get();
    return lut.reverse(bits) >> (tau - n);
}