#pragma once

#include <util/bits.hpp>
#include <vector/int_buffer.hpp>

#include <util/debug.hpp>

#include <cstddef>
#include <cstdint>
#include <tuple>

// Manages a sequence of bit_encodings
// Used as input for arbitrary shape WTs
// 
// I've decided to only support huffman- or huffman like encodings, i.e. all bits are shifted to the left of the tree
// while I'm not terribly sure what the optimal interface is for the tree construction algorithm
// this reduces a lot of headache
//
// This vector will act as the main buffer data structure for each level, just as packed_lists AND 
// std::vectors would in the unencoded cause
// The size will therefore decrease with each level
// This means that I will construct a data structure with the ability to hold up to N values
// but in practice, I will be writing less and less with each level
// Therefore, this vector has an "effective size" that decreases for each level
//
// After some deliberations, here are the requirements this thing needs to have
// 1.Every element should take a constant amount of space in the vector
//   While i strictly only need forward iterators for writing and reading packed lists
//   this makes it a lot simpler and reduces a lot of headache - at least for small levels, 
//   I know that every element is bounded by at most tau bits
// 2.I need to be able to chomp up to N elements for packed lists, and tau bits of each element at once
//   when building the packed lists 
// 3.When processing a packed list of up to N elements, it just gets loaded as a block into a machine register again
//   this means I need to make it support SIMD types
// 4.This also means that I'm probably better served by basically storing the lengths of each symbol separately from 
//   the symbols themselves, since that ensures that enough values can fit into my simd type - otherwise, I'd probably have to reduce the effective size to N/2
// 5.I probably have to store the lengths of each symbol in the vector here, even though I could probably just look it up directly on the (Huffman) tree I build
//   Of course that means I don't just have to rewrite the buffer of values with each level, but the lenghts too
//   On the other hand, not doing it means I would need to explicitly look up the huffman tree for the length of each symbol - for simd as well
//   On the other other hand, maybe if I abuse the fact it's a canonical huffman tree, I can save that as well. Idk yet
//   Note also that similar to []-indexers on other vectors, I can still return some compound object that bundles the length of a code with the code itself
//   with no runtime overhead
// Yeah.
template <typename encoded_value_type = uint64_t, size_t v_tau = sizeof(encoded_value_type) * 8>
class encoded_vector {
public:
    constexpr static size_t tau = v_tau;
    constexpr static size_t N = (sizeof(encoded_value_type) * 8) / tau;
    constexpr static size_t SIZE_BITS_PER_ELEM = log2_ceil(tau);

private:
    int_buffer<encoded_value_type> m_data;
    int_buffer<encoded_value_type> m_sizes;
    static_assert(tau < 256, "Only support up to 256 bits per element!");
    size_t m_size;

public:
    inline encoded_vector(size_t capacity) :
        m_data((capacity / N) + 1),
        m_sizes((capacity / N) + 1),
        m_size(capacity) {}

    inline void resize(size_t size) {
        m_size = size;
    }

    inline size_t size() const {
        return m_size;
    }

    inline size_t capacity() const {
        return m_data.size() * N;
    }

    inline bool empty() const {
        return size() == 0;
    }

    inline std::tuple<encoded_value_type, encoded_value_type> get(size_t i, size_t n = 1) const {
        size_t word = i / N;
        size_t offs = i % N;
        assert(offs + n <= N);  // Similar to packed lists, we do not allow overruns for the sake of simplicty

        encoded_value_type vals = m_data[word] >> (tau * offs);
        encoded_value_type sizes = m_sizes[word] >> (tau * offs);

        return { vals & mask(n * tau), sizes & mask(n * tau) };
    }

    int_buffer<encoded_value_type>& data() {
        return m_data;
    }

    const int_buffer<encoded_value_type>& data() const {
        return m_data;
    }

    int_buffer<encoded_value_type>& lengths() {
        return m_sizes;
    }

    const int_buffer<encoded_value_type>& lengths() const {
        return m_sizes;
    }
};