#pragma once

// Asorted debug stuff I find I need more often

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <string>
#include <sstream>

#include <immintrin.h>

// Normally, I don't rely on nested includes, but this time, debug.hpp explicitly pulls the following headers
#include <cassert>
#include <iostream>

template <typename value_type = size_t>
inline std::string print_bits(value_type value, char one = '1', char zero = '0', char sep_char = '\'', size_t sep_count = 8) {
    size_t sep = 0;
    auto bits = sizeof(value_type) * 8;
    std::stringstream s;
    for (size_t i = 0; i < bits; i++) {
        s << (value & (1ULL << (bits - 1)) ? one : zero);
        value <<= 1;
        sep++;
        if (sep == sep_count && i < (bits - 1)) {
            s << sep_char;
            sep = 0;
        }

    }
    return s.str();
}

template <typename value_type>
inline std::string print_bytes(value_type value, bool little_endian = true, size_t sep_count = 1, char sep_char  = ' ') {
    size_t sep = 0;
    std::stringstream s;
    s << std::uppercase;
    s << std::hex;

    uint8_t* bytes = (uint8_t*)&value;
    for (size_t i = 0; i < sizeof(value_type); i++) {
        s << std::setfill('0');
        s << std::setw(2);
        s << (size_t)bytes[little_endian ? i : sizeof(value_type) - i - 1];
        sep++;
        if (sep == sep_count && i < (sizeof(value) - 1)) {
            s << sep_char;
            sep = 0;
        }
    }
    return s.str();
}

inline void cout_no_output() {
    // politely tell std::cout that its input is not wanted right now
    std::cout.setstate(std::ios_base::failbit);
}

inline void cout_restart_output() {
    // ok talk again
    std::cout.clear();
}