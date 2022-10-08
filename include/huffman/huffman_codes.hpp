#pragma once

#include <huffman/huffman_code.hpp>
#include <huffman/huffman_encoder.hpp>

#include <cstddef>
#include <map>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <util/debug.hpp>

template <typename value_type, typename code_generator_type>
class huffman_codes {
private:
    static constexpr bool CAN_USE_LUT = sizeof(value_type) <= 2;
    static constexpr size_t LUT_SIZE = CAN_USE_LUT ? 1ULL << (sizeof(value_type) * 8) : 0;

    std::unordered_map<value_type, huffman_code> m_codes;
    std::unique_ptr<huffman_code[]> m_codes_ptr;

    std::map<std::pair<size_t, size_t>, value_type> m_reverse_lookup;

public:
    inline huffman_codes(const std::vector<value_type>& text) {
        if constexpr (CAN_USE_LUT) {
            auto codes = code_generator_type::create_codes(text);
            m_codes_ptr = std::make_unique<huffman_code[]>(LUT_SIZE);

            // Create reverse lookup
            for (auto [ v, hcp ] : codes) {
                m_codes_ptr[v] = hcp;
                m_reverse_lookup[{ hcp.code, hcp.length }] = v;
            }       
        } else {
            m_codes = code_generator_type::create_codes(text);

            // Create reverse lookup
            for (auto [ v, hcp ] : m_codes) {
                assert(hcp.length < MAX_HUFFMAN_LENGTH);
                m_reverse_lookup[{ hcp.code, hcp.length }] = v;
            }
        }
    }

    inline huffman_code encode(value_type v) const {
        if constexpr (CAN_USE_LUT) {
            return m_codes_ptr[v];
        } else {
            assert(m_codes.contains(v));
            return m_codes.find(v)->second;
        }
    }

    inline value_type decode(huffman_code code) const {
        assert(m_reverse_lookup.contains({ code.code, code.length }));
        return m_reverse_lookup.find({ code.code, code.length })->second;
    }
};

template <typename value_type>
using huffman_codes_lwt = huffman_codes<value_type, huffman_code_generator_lwt<value_type>>;

template <typename value_type>
using huffman_codes_wm = huffman_codes<value_type, huffman_code_generator_wm<value_type>>;