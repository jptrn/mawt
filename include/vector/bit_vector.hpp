#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <util/bits.hpp>
#include <vector/int_buffer.hpp>
#include <vector/indexer.hpp>

#include <util/debug.hpp>

class bit_vector {
private:
    using bit_vector_indexer = indexer<bit_vector, bool>;

    static constexpr size_t SIZE_TYPE_BITS = 64;
    static constexpr size_t SIZE_TYPE_PATTERN_BITS = 6ULL;
    static constexpr size_t SIZE_TYPE_MASK = 0x3FULL;

    int_buffer<uint64_t> m_bits;
    size_t m_size;

    inline static size_t offset(size_t i) {
        return i & SIZE_TYPE_MASK;
    }

    inline static size_t block(size_t i) {
        return i >> SIZE_TYPE_PATTERN_BITS;
    }

public:
    // Put 2 values of extra padding
    inline bit_vector(size_t count) : m_bits(block(count) + 2), m_size(count) {}

    inline bit_vector() : bit_vector(0) {}

    inline bit_vector(const std::vector<bool>& bits) : bit_vector(bits.size()) {
        // Manual copy to make sure `bits` is at minimum size without an explicit call to `shrink_to_fit`
        // Also don't call `set` (yet) in case clearing the bit is a performance loss
        for (size_t i = 0; i < m_size; i++) {
            if ((i & SIZE_TYPE_MASK) == 0) {
                m_bits[block(i)] = 0;
            }
            m_bits[block(i)] |= (size_t) bits[i] << offset(i);
        }
    }

    inline void clear() {
        for (size_t i = 0; i < m_bits.size(); i++) {
            m_bits[i] = 0;
        }
    }

    inline void set(size_t i, bool value) {
        auto pos = block(i);
        auto bit = offset(i);

        m_bits[pos] &= ~(1ULL << bit);
        m_bits[pos] |= (size_t) value << bit;
    }

    template <size_t b, typename value_type = size_t> 
    inline void set(size_t i, value_type value) {
        static_assert(b <= sizeof(value_type) * 8);

        const size_t word = block(i);
        const size_t pos = offset(i);
        assert(word < m_bits.size());
        if (pos + b <= SIZE_TYPE_BITS) {
            const size_t clear_mask = ~(mask(b) << pos);
            m_bits[word] &= clear_mask;
            m_bits[word] |= value << pos;
        } else {
            assert(word + 1 < m_bits.size());
            const size_t bits_a = SIZE_TYPE_BITS - pos;

            const uint64_t clear_mask_lo = ~(mask(b) << pos);
            const uint64_t clear_mask_hi = ~(mask(b) >> bits_a);
            
            m_bits[word] &= clear_mask_lo;
            m_bits[word] |= value << pos;

            m_bits[word + 1] &= clear_mask_hi;
            m_bits[word + 1] |= value >> bits_a;
        }
    }

    inline bool get(size_t i) const {
        return m_bits[block(i)] & (1ULL << offset(i));
    }

    size_t size() const {
        return m_size;
    }

    inline bit_vector_indexer operator[](size_t i) {
        return bit_vector_indexer(*this, i);
    }

    inline const bit_vector_indexer operator[](size_t i) const {
        return bit_vector_indexer(*this, i);
    }

    inline int_buffer<uint64_t>& data() {
        return m_bits;
    }

    inline const int_buffer<uint64_t>& data() const {
        return m_bits;
    }
}; 