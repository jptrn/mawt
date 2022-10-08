#pragma once

#include <cstddef>
#include <cstdint>

#include <vector/encoded/encoded_vector.hpp>
#include <vector/encoded/encoded_vector_simd.hpp>
#include <vector/packed/packed_util_simd.hpp>
#include <util/bits.hpp>

#include <immintrin.h>

#include <util/debug.hpp>

// Responsible for buffering memory writes to an encode vector
template <typename encoded_value_type = uint64_t, size_t v_tau = sizeof(encoded_value_type) * 8>
class encoded_vector_writer {
private:
    // tau-bit values must evenly fit into values
    static constexpr size_t tau = v_tau;
    static_assert((sizeof(encoded_value_type) * 8) % tau == 0);

    static size_t word_idx(size_t i) {
        size_t bit_pos = i * tau;
        return bit_pos / (sizeof(encoded_value_type) * 8);
    }

    static size_t word_bit_pos(size_t i) {
        size_t bit_pos = i * tau;
        return bit_pos % (sizeof(encoded_value_type) * 8);
    }

    static size_t word_pos(size_t i) {
        return word_bit_pos(i) / tau;
    }

    static const size_t VALUES_PER_WORD = (sizeof(encoded_value_type) * 8) / tau;

    encoded_vector<encoded_value_type, tau>& m_data;
    size_t m_start;
    size_t m_end;
    size_t m_idx;

    encoded_value_type m_cur_data;
    encoded_value_type m_cur_length;

    inline encoded_value_type init_cur() {
        return 0;
    }

public:

    inline encoded_vector_writer(encoded_vector<encoded_value_type, tau>& encoded, size_t start, size_t end) :
        m_data(encoded),
        m_start(start),
        m_end(end),
        m_idx(start),
        m_cur_data(init_cur()),
        m_cur_length(init_cur()) {}

    inline encoded_vector_writer(encoded_vector<encoded_value_type, tau>& encoded) 
        : encoded_vector_writer(encoded, 0, encoded.size()) {}

    inline void write(size_t n, encoded_value_type value, encoded_value_type lengths) {
        assert(m_end <= m_data.size() && m_idx + n <= m_end);

        value &= mask(n * tau);
        lengths &= mask(n * tau);
        
        // Buffers 1 or more value writes and flushes them to memory if the current word is full
        // When flushing, we need to account for the possibility that two intervals occupy the same word
        m_cur_data |= value << word_bit_pos(m_idx);
        m_cur_length |= lengths << word_bit_pos(m_idx);

        if (word_pos(m_idx) + n >= VALUES_PER_WORD) {
            if (word_idx(m_idx) == word_idx(m_start) && word_pos(m_start) > 0) {
                m_data.data()[word_idx(m_idx)] &= mask(word_bit_pos(m_start));
                m_data.data()[word_idx(m_idx)] |= m_cur_data;

                m_data.lengths()[word_idx(m_idx)] &= mask(word_bit_pos(m_start));
                m_data.lengths()[word_idx(m_idx)] |= m_cur_length;
            } else {
                m_data.data()[word_idx(m_idx)] = m_cur_data;
                m_data.lengths()[word_idx(m_idx)] = m_cur_length;
            }
            m_cur_data = word_bit_pos(m_idx) == 0 ? 0 : value >> ((sizeof(encoded_value_type) * 8) - word_bit_pos(m_idx));
            m_cur_length = word_bit_pos(m_idx) == 0 ? 0 : lengths >> ((sizeof(encoded_value_type) * 8) - word_bit_pos(m_idx));
        }
        
        m_idx += n;

        // If we've reached the end, flush
        if (m_idx >= m_end && word_pos(m_idx) != 0) { // Nothing to flush if word pos 0
            encoded_value_type last = m_data.data()[word_idx(m_idx)];
            encoded_value_type last_length = m_data.lengths()[word_idx(m_idx)];
            if (word_idx(m_start) == word_idx(m_end)) {
                last &= ~mask(word_bit_pos(m_end)) | mask(word_bit_pos(m_start));
                last_length &= ~mask(word_bit_pos(m_end)) | mask(word_bit_pos(m_start));
            } else {
                last &= ~mask(word_bit_pos(m_end));
                last_length &= ~mask(word_bit_pos(m_end));
            }
            m_data.data()[word_idx(m_idx)] = last | m_cur_data;
            m_data.lengths()[word_idx(m_idx)] = last_length | m_cur_length;
        }
    }

    inline void write(size_t value, size_t lengths) {
        write(1, value, lengths);
    }
};

template <>
inline __m128i encoded_vector_writer<__m128i, 8>::init_cur() {
    return _mm_setzero_si128();
}

template <>
inline __m256i encoded_vector_writer<__m256i, 8>::init_cur() {
    return _mm256_setzero_si256();
}

template <>
inline __m512i encoded_vector_writer<__m512i, 8>::init_cur() {
    return _mm512_setzero_si512();
}

template <>
inline void encoded_vector_writer<__m128i, 8>::write(size_t n, __m128i value, __m128i lengths) {
    value = mask_8(value, n);
        
    m_cur_data = _mm_or_si128(m_cur_data, sl_8(value, word_pos(m_idx)));
    m_cur_length = _mm_or_si128(m_cur_length, sl_8(lengths, word_pos(m_idx)));
    if (word_pos(m_idx) + n >= VALUES_PER_WORD) {
        if (word_idx(m_idx) == word_idx(m_start) && word_pos(m_start) > 0) {
            __m128i write = m_data.data()[word_idx(m_idx)];
            __m128i write_lengths = m_data.lengths()[word_idx(m_idx)];
            write = mask_8(write, word_pos(m_start));
            write_lengths = mask_8(write_lengths, word_pos(m_start));
            m_data.data()[word_idx(m_idx)] = _mm_or_si128(write, m_cur_data);
            m_data.lengths()[word_idx(m_idx)] = _mm_or_si128(write_lengths, m_cur_length);
        } else {
            m_data.data()[word_idx(m_idx)] = m_cur_data;
            m_data.lengths()[word_idx(m_idx)] = m_cur_length;
        }
        m_cur_data = sr_8(value, 16 - word_pos(m_idx));
        m_cur_length = sr_8(lengths, 16 - word_pos(m_idx));
    }
    
    m_idx += n;

    if (m_idx >= m_end && word_pos(m_idx) != 0) { // Nothing to flush if word pos 0
        __m128i last = m_data.data()[word_idx(m_idx)];
        __m128i last_length = m_data.lengths()[word_idx(m_idx)];
        if (word_idx(m_start) == word_idx(m_end)) {
            last = _mm_or_si128(mask_lo_8(last, word_pos(m_end)), mask_8(last, word_pos(m_start)));
            last_length = _mm_or_si128(mask_lo_8(last_length, word_pos(m_end)), mask_8(last_length, word_pos(m_start)));
        } else {
            last = mask_lo_8(last, word_pos(m_end));
            last_length = mask_lo_8(last_length, word_pos(m_end));
        }
        m_data.data()[word_idx(m_idx)] = _mm_or_si128(last, m_cur_data);
        m_data.lengths()[word_idx(m_idx)] = _mm_or_si128(last_length, m_cur_length);
    }
}

template <>
inline void encoded_vector_writer<__m256i, 8>::write(size_t n, __m256i value, __m256i lengths) {
    value = mask_8(value, n);
        
    m_cur_data = _mm256_or_si256(m_cur_data, sl_8(value, word_pos(m_idx)));
    m_cur_length = _mm256_or_si256(m_cur_length, sl_8(lengths, word_pos(m_idx)));
    if (word_pos(m_idx) + n >= VALUES_PER_WORD) {
        if (word_idx(m_idx) == word_idx(m_start) && word_pos(m_start) > 0) {
            __m256i write = m_data.data()[word_idx(m_idx)];
            __m256i write_lengths = m_data.lengths()[word_idx(m_idx)];
            write = mask_8(write, word_pos(m_start));
            write_lengths = mask_8(write_lengths, word_pos(m_start));
            m_data.data()[word_idx(m_idx)] = _mm256_or_si256(write, m_cur_data);
            m_data.lengths()[word_idx(m_idx)] = _mm256_or_si256(write_lengths, m_cur_length);
        } else {
            m_data.data()[word_idx(m_idx)] = m_cur_data;
            m_data.lengths()[word_idx(m_idx)] = m_cur_length;
        }
        m_cur_data = sr_8(value, 32 - word_pos(m_idx));
        m_cur_length = sr_8(lengths, 32 - word_pos(m_idx));
    }
    
    m_idx += n;

    if (m_idx >= m_end && word_pos(m_idx) != 0) { // Nothing to flush if word pos 0
        __m256i last = m_data.data()[word_idx(m_idx)];
        __m256i last_length = m_data.lengths()[word_idx(m_idx)];
        if (word_idx(m_start) == word_idx(m_end)) {
            last = _mm256_or_si256(mask_lo_8(last, word_pos(m_end)), mask_8(last, word_pos(m_start)));
            last_length = _mm256_or_si256(mask_lo_8(last_length, word_pos(m_end)), mask_8(last_length, word_pos(m_start)));
        } else {
            last = mask_lo_8(last, word_pos(m_end));
            last_length = mask_lo_8(last_length, word_pos(m_end));
        }
        m_data.data()[word_idx(m_idx)] = _mm256_or_si256(last, m_cur_data);
        m_data.lengths()[word_idx(m_idx)] = _mm256_or_si256(last_length, m_cur_length);
    }
}

template <>
inline void encoded_vector_writer<__m512i, 8>::write(size_t n, __m512i value, __m512i lengths) {
    value = mask_8(value, n);
        
    m_cur_data = _mm512_or_si512(m_cur_data, sl_8(value, word_pos(m_idx)));
    m_cur_length = _mm512_or_si512(m_cur_length, sl_8(lengths, word_pos(m_idx)));
    if (word_pos(m_idx) + n >= VALUES_PER_WORD) {
        if (word_idx(m_idx) == word_idx(m_start) && word_pos(m_start) > 0) {
            __m512i write = m_data.data()[word_idx(m_idx)];
            __m512i write_lengths = m_data.lengths()[word_idx(m_idx)];
            write = mask_8(write, word_pos(m_start));
            write_lengths = mask_8(write_lengths, word_pos(m_start));
            m_data.data()[word_idx(m_idx)] = _mm512_or_si512(write, m_cur_data);
            m_data.lengths()[word_idx(m_idx)] = _mm512_or_si512(write_lengths, m_cur_length);
        } else {
            m_data.data()[word_idx(m_idx)] = m_cur_data;
            m_data.lengths()[word_idx(m_idx)] = m_cur_length;
        }
        m_cur_data = sr_8(value, 64 - word_pos(m_idx));
        m_cur_length = sr_8(lengths, 64 - word_pos(m_idx));
    }
    
    m_idx += n;

    if (m_idx >= m_end && word_pos(m_idx) != 0) { // Nothing to flush if word pos 0
        __m512i last = m_data.data()[word_idx(m_idx)];
        __m512i last_length = m_data.lengths()[word_idx(m_idx)];
        if (word_idx(m_start) == word_idx(m_end)) {
            last = _mm512_or_si512(mask_lo_8(last, word_pos(m_end)), mask_8(last, word_pos(m_start)));
            last_length = _mm512_or_si512(mask_lo_8(last_length, word_pos(m_end)), mask_8(last_length, word_pos(m_start)));
        } else {
            last = mask_lo_8(last, word_pos(m_end));
            last_length = mask_lo_8(last_length, word_pos(m_end));
        }
        m_data.data()[word_idx(m_idx)] = _mm512_or_si512(last, m_cur_data);
        m_data.lengths()[word_idx(m_idx)] = _mm512_or_si512(last_length, m_cur_length);
    }
}

template <>
inline void encoded_vector_writer<__m128i, 8>::write(size_t value, size_t lengths) {
    write(1, _mm_set_epi64((__m64)0ULL, (__m64)value), _mm_set_epi64((__m64)0ULL, (__m64)lengths));
}

template <>
inline void encoded_vector_writer<__m256i, 8>::write(size_t value, size_t lengths) {
    write(1, _mm256_set_epi64x(0, 0, 0, value), _mm256_set_epi64x(0, 0, 0, lengths));
}

template <>
inline void encoded_vector_writer<__m512i, 8>::write(size_t value, size_t lengths) {
    write(1, _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, value), _mm512_set_epi64(0, 0, 0, 0, 0, 0, 0, lengths));
}
