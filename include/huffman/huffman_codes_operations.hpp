#pragma once

#include <utility>

#include <huffman/huffman_encoder.hpp>
#include <huffman/huffman_code.hpp>
#include <huffman/huffman_codes.hpp>
#include <vector/bit_vector_operations.hpp>

#include <util/debug.hpp>

template <typename value_type>
inline value_type huffman_codes_decode(const huffman_codes_lwt<value_type>& codes, const bit_vector& bv) {
    huffman_code hcp;
    hcp.code = bit_vector_to<size_t>(bv) << (MAX_HUFFMAN_LENGTH - bv.size());
    hcp.length = bv.size();
    return codes.decode(hcp);
}

template <typename value_type>
inline value_type huffman_codes_decode(const huffman_codes_wm<value_type>& codes, const bit_vector& bv) {
    huffman_code hcp;
    hcp.code = bit_vector_to<size_t>(bv) << (MAX_HUFFMAN_LENGTH - bv.size());
    hcp.length = bv.size();
    return codes.decode(hcp);
}