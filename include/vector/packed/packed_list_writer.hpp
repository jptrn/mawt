#pragma once

#include <cstddef>
#include <cstdint>

#include <vector/packed/packed_list.hpp>
#include <vector/packed/packed_list_simd.hpp>
#include <vector/packed/packed_util_simd.hpp>
#include <util/bits.hpp>

#include <immintrin.h>

#include <util/debug.hpp>

// Responsible for buffering memory writes to a packed list
template <size_t tau, typename packed_value_type = uint64_t>
class packed_list_writer {
private:
    // tau-bit values must evenly fit into packed_values
    static_assert((sizeof(packed_value_type) * 8) % tau == 0);

    static size_t word_idx(size_t i) {
        size_t bit_pos = i * tau;
        return bit_pos / (sizeof(packed_value_type) * 8);
    }

    static size_t word_bit_pos(size_t i) {
        size_t bit_pos = i * tau;
        return bit_pos % (sizeof(packed_value_type) * 8);
    }

    static size_t word_pos(size_t i) {
        return word_bit_pos(i) / tau;
    }

    static const size_t VALUES_PER_WORD = (sizeof(packed_value_type) * 8) / tau;

    packed_list<tau, packed_value_type>& m_list;
    size_t m_start;
    size_t m_end;
    size_t m_idx;

    packed_value_type m_cur;

    inline packed_value_type init_cur() {
        return 0;
    }

public:

    inline packed_list_writer(packed_list<tau, packed_value_type>& list, size_t start, size_t end) :
        m_list(list),
        m_start(start),
        m_end(end),
        m_idx(start),
        m_cur(init_cur()) {}

    inline packed_list_writer(packed_list<tau, packed_value_type>& list) 
        : packed_list_writer(list, 0, list.size()) {}

    inline void write(size_t value) {
        write(1, value);
    }

    inline void write(size_t n, packed_value_type value) {
        value &= mask(n * tau);
        
        // Buffers 1 or more value writes and flushes them to memory if the current word is full
        // When flushing, we need to account for the possibility that two intervals occupy the same word
        m_cur |= value << word_bit_pos(m_idx);
        if (word_pos(m_idx) + n >= VALUES_PER_WORD) {
            if (word_idx(m_idx) == word_idx(m_start) && word_pos(m_start) > 0) {
                m_list.data()[word_idx(m_idx)] &= mask(word_bit_pos(m_start));
                m_list.data()[word_idx(m_idx)] |= m_cur;
            } else {
                m_list.data()[word_idx(m_idx)] = m_cur;
            }
            m_cur = word_bit_pos(m_idx) == 0 ? 0 : value >> ((sizeof(packed_value_type) * 8) - word_bit_pos(m_idx));
        }
        
        m_idx += n;

        // If we've reached the end, flush
        if (m_idx >= m_end && word_pos(m_idx) != 0) { // Nothing to flush if word pos 0
            packed_value_type last = m_list.data()[word_idx(m_idx)];
            if (word_idx(m_start) == word_idx(m_end)) {
                last &= ~mask(word_bit_pos(m_end)) | mask(word_bit_pos(m_start));
            } else {
                last &= ~mask(word_bit_pos(m_end));
            }
            m_list.data()[word_idx(m_idx)] = last | m_cur;
        }
    }
};

template <>
inline __m128i packed_list_writer<8, __m128i>::init_cur() {
    return _mm_setzero_si128();
}

template <>
inline __m256i packed_list_writer<8, __m256i>::init_cur() {
    return _mm256_setzero_si256();
}

template <>
inline __m512i packed_list_writer<8, __m512i>::init_cur() {
    return _mm512_setzero_si512();
}


template <>
inline void packed_list_writer<8, __m128i>::write(size_t n, __m128i value) {
    value = mask_8(value, n);
        
    m_cur = _mm_or_si128(m_cur, sl_8(value, word_pos(m_idx)));
    if (word_pos(m_idx) + n >= VALUES_PER_WORD) {
        if (word_idx(m_idx) == word_idx(m_start) && word_pos(m_start) > 0) {
            __m128i write = m_list.data()[word_idx(m_idx)];
            write = mask_8(write, word_pos(m_start));
            m_list.data()[word_idx(m_idx)] = _mm_or_si128(write, m_cur);
        } else {
            m_list.data()[word_idx(m_idx)] = m_cur;
        }
        m_cur = sr_8(value, 16 - word_pos(m_idx));
    }
    
    m_idx += n;

    if (m_idx >= m_end && word_pos(m_idx) != 0) { // Nothing to flush if word pos 0
        __m128i last = m_list.data()[word_idx(m_idx)];
        if (word_idx(m_start) == word_idx(m_end)) {
            last = _mm_or_si128(mask_lo_8(last, word_pos(m_end)), mask_8(last, word_pos(m_start)));
        } else {
            last = mask_lo_8(last, word_pos(m_end));
        }
        m_list.data()[word_idx(m_idx)] = _mm_or_si128(last, m_cur);
    }
}

template <>
inline void packed_list_writer<8, __m256i>::write(size_t n, __m256i value) {
    value = mask_8(value, n);
        
    m_cur = _mm256_or_si256(m_cur, sl_8(value, word_pos(m_idx)));
    if (word_pos(m_idx) + n >= VALUES_PER_WORD) {
        if (word_idx(m_idx) == word_idx(m_start) && word_pos(m_start) > 0) {
            __m256i write = m_list.data()[word_idx(m_idx)];
            write = mask_8(write, word_pos(m_start));
            m_list.data()[word_idx(m_idx)] = _mm256_or_si256(write, m_cur);
        } else {
            m_list.data()[word_idx(m_idx)] = m_cur;
        }
        m_cur = sr_8(value, 32 - word_pos(m_idx));
    }
    
    m_idx += n;

    if (m_idx >= m_end && word_pos(m_idx) != 0) { // Nothing to flush if word pos 0
        __m256i last = m_list.data()[word_idx(m_idx)];
        if (word_idx(m_start) == word_idx(m_end)) {
            last = _mm256_or_si256(mask_lo_8(last, word_pos(m_end)), mask_8(last, word_pos(m_start)));
        } else {
            last = mask_lo_8(last, word_pos(m_end));
        }
        m_list.data()[word_idx(m_idx)] = _mm256_or_si256(last, m_cur);
    }
}

template <>
inline void packed_list_writer<8, __m512i>::write(size_t n, __m512i value) {
    value = mask_8(value, n);
        
    m_cur = _mm512_or_si512(m_cur, sl_8(value, word_pos(m_idx)));
    if (word_pos(m_idx) + n >= VALUES_PER_WORD) {
        if (word_idx(m_idx) == word_idx(m_start) && word_pos(m_start) > 0) {
            __m512i write = m_list.data()[word_idx(m_idx)];
            write = mask_8(write, word_pos(m_start));
            m_list.data()[word_idx(m_idx)] = _mm512_or_si512(write, m_cur);
        } else {
            m_list.data()[word_idx(m_idx)] = m_cur;
        }
        m_cur = sr_8(value, 64 - word_pos(m_idx));
    }
    
    m_idx += n;

    if (m_idx >= m_end && word_pos(m_idx) != 0) { // Nothing to flush if word pos 0
        __m512i last = m_list.data()[word_idx(m_idx)];
        if (word_idx(m_start) == word_idx(m_end)) {
            last = _mm512_or_si512(mask_lo_8(last, word_pos(m_end)), mask_8(last, word_pos(m_start)));
        } else {
            last = mask_lo_8(last, word_pos(m_end));
        }
        m_list.data()[word_idx(m_idx)] = _mm512_or_si512(last, m_cur);
    }
}

template <>
inline void packed_list_writer<8, __m128i>::write(size_t value) {
    write(1, _mm_set_epi64((__m64)0ULL, (__m64)value));
}

template <>
inline void packed_list_writer<8, __m256i>::write(size_t value) {
    write(1, _mm256_set_epi64x(0, 0, 0, value));
}

template <>
inline void packed_list_writer<8, __m512i>::write(size_t value) {
    write(1, _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, value));
}
