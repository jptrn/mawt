#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <util/bits.hpp>
#include <vector/int_buffer.hpp>

#include <util/debug.hpp>

// A packed list is basically a templated int_vector with a bit width tau
// Can read and write up to N values at a time using the pack and set_list functions
// See `packed_list_simd.hpp` for specializations for larger value types
// Use packed_list_writer.hpp for writing packed_values

template <size_t tau, typename packed_value_type = uint64_t>
class packed_list {
private:
    static const size_t BITS_PER_WORD = sizeof(packed_value_type) * 8;
    static_assert(BITS_PER_WORD % tau == 0);

    inline constexpr static size_t bitsize(size_t value) {
        return value * tau;
    }

    inline static size_t wordsize(size_t value) {
        auto bits = bitsize(value);
        return (bits / BITS_PER_WORD) + 1;  // add extra space at the end to ensure we can write 512-bit values past the end
    }

    int_buffer<packed_value_type> m_data;
    size_t m_size = 0;

public:
    inline int_buffer<packed_value_type>& data() {
        return m_data;
    }

    inline packed_list(size_t size) : m_data(wordsize(size)), m_size(size) {}

    inline packed_value_type pack_aligned(size_t i, size_t n) const {        
        const size_t word = bitsize(i) / BITS_PER_WORD;
        assert((bitsize(i) % BITS_PER_WORD) == 0);
        
        const size_t total_bits = n * tau;
        return m_data[word] & mask(total_bits);
    }

    inline packed_value_type pack(size_t i, size_t n) const {        
        const size_t word = bitsize(i) / BITS_PER_WORD;
        const size_t pos = bitsize(i) % BITS_PER_WORD;
        
        const size_t total_bits = n * tau;
        assert(pos + total_bits <= BITS_PER_WORD);
        return (m_data[word] >> pos) & mask(total_bits);
    }

    inline size_t size() const {
        return m_size;
    }

    inline bool empty() const {
        return size() == 0;
    }
};
