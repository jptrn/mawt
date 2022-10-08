#pragma once

#include <huffman/huffman_code.hpp>
#include <huffman/huffman_tree.hpp>
#include <util/bits.hpp>
#include <vector/encoded/encoded_vector.hpp>
#include <vector/encoded/encoded_vector_writer.hpp>

#include <algorithm>
#include <deque>
#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>

#include <util/debug.hpp>

constexpr size_t MAX_HUFFMAN_LENGTH = 64;

template <typename value_type>
inline huffman_tree* get_huffman_tree(const std::vector<value_type>& text) {
    std::unordered_map<value_type, size_t> freq;
    for (auto c : text) {
        freq[c]++;
    }

    std::deque<huffman_tree*> codes(freq.size());
    size_t i  = 0;
    for (auto [ c, f ] : freq) {
        codes[i++] = new huffman_tree(c, f);
    }
    std::sort(codes.begin(), codes.end(), [](const huffman_tree* a, const huffman_tree* b) {
        return a->freq() < b->freq();
    });

    while (codes.size() > 1) {
        auto ca = codes.front();
        codes.pop_front();

        auto cb = codes.front();
        codes.pop_front();
  
        auto parent = new huffman_tree(ca, cb);

        // Insert parent node, maintaining sort order by frequency
        for (size_t i = 0;; i++) {
            if (i == codes.size() || codes[i]->freq() >= parent->freq()) {
                codes.insert(codes.begin() + i, parent);
                break;
            }
        }
    }

    return codes.front();
}

template <typename value_type>
void get_huffman_codes(const huffman_tree* h, size_t code, size_t length, size_t& max_length, std::unordered_map<value_type, huffman_code>& codes) {
    if (h->left()) {
        get_huffman_codes<value_type>(h->left(), code, length + 1, max_length, codes);
    }
    if (h->right()) {
        get_huffman_codes<value_type>(h->right(), code | (1ULL << length), length + 1, max_length, codes);
    }

    if (h->is_leaf()) {
        max_length = std::max(length, max_length);
        codes[h->code()] = { code, length };
    }
}

template <typename value_type>
std::unordered_map<value_type, huffman_code> get_huffman_codes(const huffman_tree* h, size_t& max_length) {
    std::unordered_map<value_type, huffman_code> codes;
    max_length = 0;
    get_huffman_codes<value_type>(h, 0, 0, max_length, codes);
    return codes;
}

template <typename value_type>
std::unordered_map<value_type, huffman_code> get_canonical_huffman_codes(const std::unordered_map<value_type, huffman_code>& codes) {
    // Create canonical codes - need to be highest significant bit first
    std::vector<std::tuple<value_type, huffman_code>> canonical_codes;
    for (auto [ c, v ] : codes) {
        canonical_codes.push_back({ c, v });
    }

    // Sort codes by length, ascending
    std::sort(canonical_codes.begin(), canonical_codes.end(), [](const auto& a, const auto& b) {
        return std::get<1>(a).length < std::get<1>(b).length;
    });

    size_t new_code = 0;
    size_t new_len = std::get<1>(canonical_codes[0]).length;
    for (auto& c : canonical_codes) {
        auto& [ ch, code ] = c;
        auto& [ bits, len ] = code;

        new_code <<= len - new_len;
        new_len = len;

        bits = new_code;

        new_code++; 
    }

    std::unordered_map<value_type, huffman_code> retval;
    for (auto [ c, v ] : canonical_codes) {
        retval[c] = v;
    }
    return retval;
}

template <typename value_type>
inline std::unordered_map<value_type, huffman_code> get_huffman_encoding_canonical(const std::vector<value_type>& text, size_t& max_length) {
    auto tree = std::unique_ptr<huffman_tree>(get_huffman_tree(text));
    auto codes = get_huffman_codes<value_type>(tree.get(), max_length);
    return get_canonical_huffman_codes<value_type>(codes);
}

template <typename value_type>
class huffman_code_generator_lwt {
private:
    inline huffman_code_generator_lwt() {}

public:
    inline static std::unordered_map<value_type, huffman_code> create_codes(const std::vector<value_type>& text) {
        size_t max_length = 0;
        auto canonical_codes = get_huffman_encoding_canonical<value_type>(text, max_length);

        // Prepare for LWT representation, align with MSB and invert
        for (auto& [ c, v ] : canonical_codes) {
            auto [ code, length ] = v;
            v = { ~code << (MAX_HUFFMAN_LENGTH - length), length };
        }
        
        return canonical_codes;
    }
};


template <typename value_type>
class huffman_code_generator_wm {
private:
    inline huffman_code_generator_wm() {}

public:
    inline static std::unordered_map<value_type, huffman_code> create_codes(const std::vector<value_type>& text) {
        size_t max_length = 0;
        auto canonical_codes = get_huffman_encoding_canonical<value_type>(text, max_length);
        std::deque<size_t> valid_codes { 1, 0 };

        // Prepare for WM representation
        std::unordered_map<size_t, std::vector<std::tuple<value_type, huffman_code>>> code_lengths;
        for (auto [ c, v ] : canonical_codes) {
            auto [ code, length ] = v;
            code_lengths[length].push_back({ c, v });
        } 

        for (size_t j = 1; j <= max_length; j++) {
            // Extract all codes of length j
            for (auto& [ c, v ] : code_lengths[j]) {
                size_t new_code = valid_codes.front();
                valid_codes.pop_front();
                canonical_codes[c] = { new_code << (MAX_HUFFMAN_LENGTH - j), j };
            }

            size_t codes_count = valid_codes.size();
            for (size_t i = 0; i < codes_count; i++) {
                size_t code = valid_codes[i];
                valid_codes[i] = (code << 1) | 1;   // Insert codes pre-sorted
                valid_codes.push_back(code << 1);   // saves the sorting pass later
            }
        }
        return canonical_codes;
    }
};