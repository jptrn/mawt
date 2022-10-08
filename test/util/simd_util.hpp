#pragma once

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

#include <util/debug.hpp>

#include <immintrin.h>

template <typename value_type>
void compare_eq(value_type a, value_type b) {
    uint8_t* av = (uint8_t*)&a; // I'm pretty sure this is UB
    uint8_t* bv = (uint8_t*)&b;
    for (size_t i = 0; i < sizeof(value_type); i++) {
        ASSERT_EQ(av[i], bv[i]) << " mismatch:" << std::endl
            << print_bytes(a, false) << std::endl
            << "vs. " << std::endl
            << print_bytes(b, false);
    }
}

#define ASSERT_SIMD_EQ(a, b) ASSERT_NO_FATAL_FAILURE(compare_eq(a, b))