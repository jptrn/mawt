#pragma once

#include <cstddef>

#include <util/bits.hpp>
#include <util/bit_util_simd.hpp>
#include <vector/bit_vector.hpp>
#include <wt/parallel/dd_merge_bv.hpp>

#include <util/debug.hpp>

#include <immintrin.h>

// 128 Bits

template <>
inline void copy_interval_words<__m128i>(const bit_vector& src, bit_vector& target, size_t write_start, size_t write_end, size_t read_i) {
    // Only copy whole words, we'll deal with the boundaries later
    const size_t write_start_word = divceil(write_start, 128);
    read_i += write_start_word * 128 - write_start; // Skip the first non-aligned bits
    const size_t write_end_word = write_end / 128;
    const size_t read_bit_offset = read_i % 128;

    const __m128i* read = (__m128i*)src.data().data();
    __m128i* write = (__m128i*)target.data().data();

    size_t word_read_i = read_i / 128;
    __m128i temp = sr_bits(read[word_read_i++], read_bit_offset);
    for (size_t write_word = write_start_word; write_word < write_end_word; write_word++) {
        if (read_bit_offset != 0) {
            __m128i next = read[word_read_i++];
            write[write_word] = _mm_or_si128(temp, sl_bits(next, 128 - read_bit_offset));
            temp = sr_bits(next, read_bit_offset);
        } else {
            write[write_word] = temp;
            temp = read[word_read_i++];
        }
    }
}

template <>
inline __m128i read_bits<__m128i>(const bit_vector& src, size_t i, size_t n) {
    const __m128i* read = (__m128i*)src.data().data();
    assert(n <= 128);

    size_t read_word = i / 128;
    size_t read_bit = i % 128;

    __m128i temp = sr_bits(read[read_word], read_bit);
    if (read_bit + n > 128) {
        __m128i next = sl_bits(read[read_word + 1], 128 - read_bit);
        temp = _mm_or_si128(temp, next);
    }
    return mask_bits(temp, n);
}

template <>
inline void copy_interval_boundaries<__m128i>(const bit_vector& src, bit_vector& target, size_t write_start, size_t write_end, size_t read_i) {
    const size_t write_start_word = write_start / 128;
    const size_t write_end_word = write_end / 128;
    const size_t write_bit_offset = write_start % 128;
    __m128i* write = (__m128i*)target.data().data();

    size_t read_count = write_end - write_start;
    if (write_start_word == write_end_word) {
        assert(write_bit_offset + read_count < 128);
        __m128i word = sl_bits(read_bits<__m128i>(src, read_i, read_count), write_bit_offset);
        write[write_start_word] = _mm_or_si128(write[write_start_word], word);
    } else {
        size_t read_count_start = 128 - write_bit_offset;
        size_t read_count_end = write_end % 128;

        __m128i word_start = sl_bits(read_bits<__m128i>(src, read_i, read_count_start), write_bit_offset);
        write[write_start_word] = _mm_or_si128(write[write_start_word], word_start);
        __m128i word_end = read_bits<__m128i>(src, read_i + read_count - read_count_end, read_count_end);
        write[write_end_word] = _mm_or_si128(write[write_end_word], word_end);
    }
}

template <>
inline size_t count_bits<__m128i>(const bit_vector& src, size_t start, size_t end) {
    const __m128i* read = (__m128i*)src.data().data();

    size_t read_word = start / 128;
    size_t read_bit = start % 128;
    size_t end_word = end / 128;
    size_t end_bit = end % 128;

    if (read_word == end_word) {
        __m128i word = sr_bits(read[read_word], read_bit);
        __m128i count = _mm_popcnt_epi64(mask_bits(word, end - start));
        return _mm_extract_epi64(count, 0) + _mm_extract_epi64(count, 1);
    }
    
    // Read first (possibly) partial word
    __m128i word = sr_bits(read[read_word++], read_bit);
    __m128i count = _mm_popcnt_epi64(word);

    // read whole words
    for (; read_word < end_word; read_word++) {
        count = _mm_add_epi64(count, _mm_popcnt_epi64(read[read_word]));
    }

    // Read final word, possibly empty
    __m128i final = mask_bits(read[end_word], end_bit);
    count = _mm_add_epi64(count, _mm_popcnt_epi64(final));
    return _mm_extract_epi64(count, 0) + _mm_extract_epi64(count, 1);
}

// 256 Bits

template <>
inline void copy_interval_words<__m256i>(const bit_vector& src, bit_vector& target, size_t write_start, size_t write_end, size_t read_i) {
    // Only copy whole words, we'll deal with the boundaries later
    const size_t write_start_word = divceil(write_start, 256);
    read_i += write_start_word * 256 - write_start; // Skip the first non-aligned bits
    const size_t write_end_word = write_end / 256;
    const size_t read_bit_offset = read_i % 256;

    const __m256i* read = reinterpret_cast<const __m256i*>(src.data().data());
    __m256i* write = reinterpret_cast<__m256i*>(target.data().data());

    size_t word_read_i = read_i / 256;
    __m256i temp = sr_bits(_mm256_load_si256(read + word_read_i++), read_bit_offset);
    for (size_t write_word = write_start_word; write_word < write_end_word; write_word++) {
        if (read_bit_offset != 0) {
            __m256i next = _mm256_load_si256(read + word_read_i++);
            _mm256_store_si256(write + write_word, _mm256_or_si256(temp, sl_bits(next, 256 - read_bit_offset)));
            temp = sr_bits(next, read_bit_offset);
        } else {
            _mm256_store_si256(write + write_word, temp);
            temp = _mm256_load_si256(read + word_read_i++);
        }
    }
}

template <>
inline __m256i read_bits<__m256i>(const bit_vector& src, size_t i, size_t n) {
    const __m256i* read = reinterpret_cast<const __m256i*>(src.data().data());
    assert(n <= 256);

    size_t read_word = i / 256;
    size_t read_bit = i % 256;

    __m256i temp = sr_bits(_mm256_load_si256(read + read_word), read_bit);
    if (read_bit + n > 256) {
        __m256i next = sl_bits(_mm256_load_si256(read + read_word + 1), 256 - read_bit);
        temp = _mm256_or_si256(temp, next);
    }
    return mask_bits(temp, n);
}

template <>
inline void copy_interval_boundaries<__m256i>(const bit_vector& src, bit_vector& target, size_t write_start, size_t write_end, size_t read_i) {
    const size_t write_start_word = write_start / 256;
    const size_t write_end_word = write_end / 256;
    const size_t write_bit_offset = write_start % 256;
    __m256i* write = reinterpret_cast<__m256i*>(target.data().data());

    size_t read_count = write_end - write_start;
    if (write_start_word == write_end_word) {
        assert(write_bit_offset + read_count < 256);
        __m256i word = sl_bits(read_bits<__m256i>(src, read_i, read_count), write_bit_offset);
        _mm256_store_si256(write + write_start_word, _mm256_or_si256(_mm256_load_si256(write + write_start_word), word));
    } else {
        size_t read_count_start = 256 - write_bit_offset;
        size_t read_count_end = write_end % 256;

        __m256i word_start = sl_bits(read_bits<__m256i>(src, read_i, read_count_start), write_bit_offset);
        _mm256_store_si256(write + write_start_word, _mm256_or_si256(_mm256_load_si256(write + write_start_word), word_start));
        __m256i word_end = read_bits<__m256i>(src, read_i + read_count - read_count_end, read_count_end);
        _mm256_store_si256(write + write_end_word, _mm256_or_si256(_mm256_load_si256(write + write_end_word), word_end));
    }
}

template <>
inline size_t count_bits<__m256i>(const bit_vector& src, size_t start, size_t end) {
    // TODO: Align Reads
    const __m256i* read = reinterpret_cast<const __m256i*>(src.data().data());

    size_t read_word = start / 256;
    size_t read_bit = start % 256;
    size_t end_word = end / 256;
    size_t end_bit = end % 256;

    if (read_word == end_word) {
        __m256i word = sr_bits(_mm256_load_si256(read + read_word), read_bit);
        __m256i count = _mm256_popcnt_epi64(mask_bits(word, end - start));
        return _mm256_extract_epi64(count, 0) + _mm256_extract_epi64(count, 1)
             + _mm256_extract_epi64(count, 2) + _mm256_extract_epi64(count, 3);
    }
    
    // Read first (possibly) partial word
    __m256i word = sr_bits(_mm256_load_si256(read + read_word++), read_bit);
    __m256i count = _mm256_popcnt_epi64(word);
    // read whole words
    for (; read_word < end_word; read_word++) {
        count = _mm256_add_epi64(count, _mm256_popcnt_epi64(_mm256_load_si256(read + read_word)));
    }

    // Read final word, possibly empty
    __m256i final = mask_bits(_mm256_load_si256(read + end_word), end_bit);
    count = _mm256_add_epi64(count, _mm256_popcnt_epi64(final));
    return _mm256_extract_epi64(count, 0) + _mm256_extract_epi64(count, 1)
         + _mm256_extract_epi64(count, 2) + _mm256_extract_epi64(count, 3);
}


// 512 Bits

template <>
inline void copy_interval_words<__m512i>(const bit_vector& src, bit_vector& target, size_t write_start, size_t write_end, size_t read_i) {
    // Only copy whole words, we'll deal with the boundaries later
    const size_t write_start_word = divceil(write_start, 512);
    read_i += write_start_word * 512 - write_start; // Skip the first non-aligned bits
    const size_t write_end_word = write_end / 512;
    const size_t read_bit_offset = read_i % 512;

    const __m512i* read = (__m512i*)src.data().data();
    __m512i* write = (__m512i*)target.data().data();

    size_t word_read_i = read_i / 512;
    __m512i temp = sr_bits(_mm512_load_si512(read + word_read_i++), read_bit_offset);
    for (size_t write_word = write_start_word; write_word < write_end_word; write_word++) {
        if (read_bit_offset != 0) {
            __m512i next = _mm512_load_si512(read + word_read_i++);
            _mm512_store_si512(write + write_word, _mm512_or_si512(temp, sl_bits(next, 512 - read_bit_offset)));
            temp = sr_bits(next, read_bit_offset);
        } else {
            _mm512_store_si512(write + write_word, temp);
            temp = _mm512_load_si512(read + word_read_i++);
        }
    }
}

template <>
inline __m512i read_bits<__m512i>(const bit_vector& src, size_t i, size_t n) {
    const __m512i* read = (__m512i*)src.data().data();
    assert(n <= 512);

    size_t read_word = i / 512;
    size_t read_bit = i % 512;

    __m512i temp = sr_bits(_mm512_load_si512(read + read_word), read_bit);
    if (read_bit + n > 512) {
        __m512i next = sl_bits(_mm512_load_si512(read + read_word + 1), 512 - read_bit);
        temp = _mm512_or_si512(temp, next);
    }
    return mask_bits(temp, n);
}

template <>
inline void copy_interval_boundaries<__m512i>(const bit_vector& src, bit_vector& target, size_t write_start, size_t write_end, size_t read_i) {
    const size_t write_start_word = write_start / 512;
    const size_t write_end_word = write_end / 512;
    const size_t write_bit_offset = write_start % 512;
    __m512i* write = (__m512i*)target.data().data();

    size_t read_count = write_end - write_start;
    if (write_start_word == write_end_word) {
        assert(write_bit_offset + read_count < 512);
        __m512i word = sl_bits(read_bits<__m512i>(src, read_i, read_count), write_bit_offset);
        _mm512_store_si512(write + write_start_word, _mm512_or_si512(_mm512_load_si512(write + write_start_word), word));
    } else {
        size_t read_count_start = 512 - write_bit_offset;
        size_t read_count_end = write_end % 512;

        __m512i word_start = sl_bits(read_bits<__m512i>(src, read_i, read_count_start), write_bit_offset);
        _mm512_store_si512(write + write_start_word, _mm512_or_si512(_mm512_load_si512(write + write_start_word), word_start));
        __m512i word_end = read_bits<__m512i>(src, read_i + read_count - read_count_end, read_count_end);
        _mm512_store_si512(write + write_end_word, _mm512_or_si512(_mm512_load_si512(write + write_end_word), word_end));
    }
}

template <>
inline size_t count_bits<__m512i>(const bit_vector& src, size_t start, size_t end) {
    // TODO: Align Reads
    const __m512i* read = (__m512i*)src.data().data();

    size_t read_word = start / 512;
    size_t read_bit = start % 512;
    size_t end_word = end / 512;
    size_t end_bit = end % 512;

    if (read_word == end_word) {
        __m512i word = sr_bits(_mm512_load_si512(read + read_word), read_bit);
        __m512i count = _mm512_popcnt_epi64(mask_bits(word, end - start));
        return _mm512_reduce_add_epi64(count);
    }
    
    // Read first (possibly) partial word
    __m512i word = sr_bits(_mm512_load_si512(read + read_word++), read_bit);
    __m512i count = _mm512_popcnt_epi64(word);
    // read whole words
    for (; read_word < end_word; read_word++) {
        count = _mm512_add_epi64(count, _mm512_popcnt_epi64(_mm512_load_si512(read + read_word)));
    }

    // Read final word, possibly empty
    __m512i final = mask_bits(_mm512_load_si512(read + end_word), end_bit);
    count = _mm512_add_epi64(count, _mm512_popcnt_epi64(final));
    return _mm512_reduce_add_epi64(count);
}