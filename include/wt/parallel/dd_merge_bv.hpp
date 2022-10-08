#pragma once

#include <cstddef>

#include <util/bits.hpp>
#include <vector/bit_vector.hpp>

// Bit vector operations for domain decomposition, primarily memcpy and zero_count

template <typename bit_copy_type>
static inline void copy_interval_words(const bit_vector& src, bit_vector& target, size_t write_start, size_t write_end, size_t read_i) {
    const size_t bit_block_size = sizeof(bit_copy_type) * 8;

    // Only copy whole words, we'll deal with the boundaries later
    const size_t write_start_word = divceil(write_start, bit_block_size);
    read_i += write_start_word * bit_block_size - write_start; // Skip the first non-aligned bits
    const size_t write_end_word = write_end / bit_block_size;
    const size_t read_bit_offset = read_i % bit_block_size;

    // Danger Zone again
    const auto* read = (bit_copy_type*)src.data().data();
    auto* write = (bit_copy_type*)target.data().data();

    // Copy bits block-wise, basically a bit-unaligned memcpy
    // prefetch the upper bits of the previous word
    size_t word_read_i = read_i / bit_block_size;
    bit_copy_type temp = read[word_read_i++] >> read_bit_offset;
    for (size_t write_word = write_start_word; write_word < write_end_word; write_word++) {
        // fetch the next word
        if (read_bit_offset != 0) {
            bit_copy_type next = read[word_read_i++];
            temp |= next << (bit_block_size - read_bit_offset);
            write[write_word] = temp;
            temp = next >> read_bit_offset;
        } else {
            write[write_word] = temp;
            temp = read[word_read_i++];
        }
    }
}

template <typename bit_copy_type>
static inline bit_copy_type read_bits(const bit_vector& src, size_t i, size_t n) {
    const size_t bit_block_size = sizeof(bit_copy_type) * 8;
    const auto* read = (bit_copy_type*)src.data().data();
    assert(n <= bit_block_size);

    size_t read_word = i / bit_block_size;
    size_t read_bit = i % bit_block_size;

    bit_copy_type temp = read[read_word] >> read_bit;
    if (read_bit + n > bit_block_size) {
        temp |= read[read_word + 1] << (bit_block_size - read_bit);
    }
    return temp & mask(n);
}

template <typename bit_copy_type>
static inline void copy_interval_boundaries(const bit_vector& src, bit_vector& target, size_t write_start, size_t write_end, size_t read_i) {
    const size_t bit_block_size = sizeof(bit_copy_type) * 8;

    const size_t write_start_word = write_start / bit_block_size;
    const size_t write_end_word = write_end / bit_block_size;
    const size_t write_bit_offset = write_start % bit_block_size;
    auto* write = (bit_copy_type*)target.data().data();

    // We basically have the following cases
    // - The target interval occupies more than one word
    //      - We may need to copy `n` bits from the start of the interval where `n` can be 0
    //      - We may need to copy `m` bits from the end of the interval where `m` can be 0
    // - The target interval may be located fully within one word
    // - The target interval may be empty

    size_t read_count = write_end - write_start;
    if (write_start_word == write_end_word) {
        assert(write_bit_offset + read_count < bit_block_size);
        write[write_start_word] |= read_bits<bit_copy_type>(src, read_i, read_count) << write_bit_offset;
    } else {
        size_t read_count_start = bit_block_size - write_bit_offset;
        size_t read_count_end = write_end % bit_block_size;

        write[write_start_word] |= read_bits<bit_copy_type>(src, read_i, read_count_start) << write_bit_offset;
        write[write_end_word] |= read_bits<bit_copy_type>(src, read_i + read_count - read_count_end, read_count_end);
    }
}

template <typename bit_copy_type>
static inline size_t count_bits(const bit_vector& src, size_t start, size_t end) {
    const size_t bit_block_size = sizeof(bit_copy_type) * 8;
    const auto* read = (bit_copy_type*)src.data().data();

    size_t read_word = start / bit_block_size;
    size_t read_bit = start % bit_block_size;
    size_t end_word = end / bit_block_size;
    size_t end_bit = end % bit_block_size;

    if (read_word == end_word) {
        return __builtin_popcountll((read[read_word] >> read_bit) & mask(end - start));
    }
    
    // Read first (possibly) partial word
    bit_copy_type word = read[read_word++] >> read_bit;
    size_t count = __builtin_popcountll(word);

    // read whole words
    for (; read_word < end_word; read_word++) {
        count += __builtin_popcountll(read[read_word]);
    }

    // Read final word, possibly empty
    return count + __builtin_popcountll(read[end_word] & mask(end_bit));
}