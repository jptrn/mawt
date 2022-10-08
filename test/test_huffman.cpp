#include <gtest/gtest.h>

#include <huffman/huffman_encoder.hpp>
#include <huffman/huffman_codes.hpp>
#include <util/bits.hpp>

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <deque>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <tuple>

TEST(Huffman, LwtBasic) {
    std::string text = "!?. lorem ipsum dolor sit amet !!!!!!bbbaaa!!!?";
    std::unordered_map<char, size_t> histogram;
    for (auto c : text) {
        histogram[c]++;
    }
    
    std::vector<char> data(text.begin(), text.end());
    huffman_codes_lwt<char> codes(data);
    std::unordered_map<char, size_t> code_lengths;

    for (size_t i = 0; i < text.size(); i++) {
        auto hcp = codes.encode(text[i]);
        auto [ code, length ] = hcp;
        auto ch = codes.decode(hcp);
        ASSERT_EQ(ch, data[i]) << "mismatch @ " << i;

        // Check that each length matches
        if (!code_lengths.contains(data[i])) {
            code_lengths[data[i]] = length;
        }
        ASSERT_EQ(code_lengths[data[i]], length) << "mismatch for code " << code;
    }

    ASSERT_EQ(code_lengths.size(), histogram.size());
}

TEST(Huffman, LwtRepresentation) {
    std::string text = "!?. lorem ipsum dolor sit amet !!!!!!bbbaaa!!!?";
    std::vector<char> data(text.begin(), text.end());
    huffman_codes_lwt<char> codes(data);

    // We need to ensore that codes are - canonical, aligned with the MSB of char, MSB -> LSB, and inverted
    std::map<size_t, size_t> code_lengths;
    for (auto c : text) {
        auto [ code, length ] = codes.encode(c);
        code_lengths[(~code >> (8 - length)) & mask(length)] = length;
    }

    char code = 0x00;
    size_t len = 0;
    for (auto [ c, l ] : code_lengths) {
        if (c != 0) {
            ASSERT_GT(c, code);
        }
        ASSERT_GE(l, len) << "code: " << c << ", new code: " << code;
        code = c;
        len = l;
    }

    // Ensure prefix-free ness
    for (auto [ code, length ] : code_lengths) {
        for (auto [ cs, ls ] : code_lengths) {
            if (ls > length) {
                ASSERT_NE(code, cs << (ls - length)) << print_bits((char)code) << " (" << (size_t)length << ") is prefix of " << print_bits((char)cs) << " (" << (size_t)ls << ")";
            }
        }
    }
}


TEST(Huffman, WmBasic) {
    std::string text = "!?. lorem ipsum dolor sit amet !!!!!!bbbaaa!!!?";
    std::unordered_map<char, size_t> histogram;
    for (auto c : text) {
        histogram[c]++;
    }
    
    std::vector<char> data(text.begin(), text.end());
    huffman_codes_wm<char> codes(data);
    std::unordered_map<char, size_t> code_lengths;
    
    for (size_t i = 0; i < text.size(); i++) {
        auto hcp = codes.encode(text[i]);
        auto [ code, length ] = hcp;
        auto ch = codes.decode(hcp);
        ASSERT_EQ(ch, data[i]) << "mismatch @ " << i;

        // Check that each length matches
        if (!code_lengths.contains(data[i])) {
            code_lengths[data[i]] = length;
        }
        ASSERT_EQ(code_lengths[data[i]], length) << "mismatch for code " << code;
    }

    ASSERT_EQ(code_lengths.size(), histogram.size());
}

TEST(Huffman, WmRepresentation) {
    std::string text = "!?. lorem ipsum dolor sit amet !!!!!!bbbaaa!!!?";
    std::vector<char> data(text.begin(), text.end());
    huffman_codes_wm<char> codes(data);

    // We need to ensore that codes are 
    // - aligned with MSB
    // - prefix-free
    // - inverted, reverted prefixes in descending order
    std::map<size_t, std::unordered_set<size_t>> code_lengths;
    size_t max_length = 0;
    for (auto c : text) {
        auto [ code, length ] = codes.encode(c);
        code_lengths[length].emplace((code >> (MAX_HUFFMAN_LENGTH - length)) & mask(length));
        max_length = std::max(max_length, (size_t)length);
    }

    // Ensure prefix-free ness
    for (auto [ length, codes ] : code_lengths) {
        for (auto code : codes) {
            for (size_t ls = length + 1; ls <= max_length; ls++) {
                for (auto cs : code_lengths[ls]) {
                    ASSERT_NE(cs, code << (ls - length)) << print_bits((char)code) << " (" << (size_t)length << ") is prefix of " << print_bits((char)cs) << " (" << (size_t)ls << ")";
                }
            }
        }
    }
}
