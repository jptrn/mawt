#include <gtest/gtest.h>

#include <huffman/huffman_encoder.hpp>
#include <huffman/huffman_codes.hpp>
#include <huffman/huffman_codes_operations.hpp>
#include <util/bits.hpp>
#include <vector/bit_vector_operations.hpp>
#include <wt/wm_huffman.hpp>
#include <wt/wm_huffman_operations.hpp>
#include <wt/wt_params.hpp>

#include <util/bits.hpp>
#include <util/debug.hpp>
#include <util/random.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <immintrin.h>

#define RANDOM_SIZE_A 512
#define RANDOM_SIZE_B 256

template <typename value_type>
static std::string fail_info(const std::vector<value_type>& expected, const huffman_codes_wm<value_type>& codes, const wm_huffman& t, size_t i, size_t start) {
    std::stringstream s;
    auto encoded = codes.encode(expected[i]);
    s << "mismatch @ " << i << " (" << (i - start) << " in wm)" << std::endl;
    s << "[expected: " << (size_t)expected[i] << ", code: " << print_bits(encoded.code) << " (" << encoded.length << ")]" << std::endl;
    auto bv = wm_huffman_get(t, i - start);
    auto actual = bit_vector_to<size_t>(bv);
    s << "[actual: " << print_bits(actual) << " (" << bv.size() << "), decoded: " << (size_t)huffman_codes_decode<value_type>(codes, bv) << "]";
    return s.str();
}

template <typename value_type, typename wt_impl_type>
void test_wm_huffman_access(const std::vector<value_type>& expected, const huffman_codes_wm<value_type>& codes, size_t start, size_t end) {
    auto t = wm_huffman::make<value_type, wt_impl_type>(expected, codes, start, end);
    size_t bit_depth = 0;
    std::vector<size_t> depth_count;
    for (size_t i = start; i < end; i++) {
        auto ch = expected[i];
        auto [ code, length ] = codes.encode(ch);
        bit_depth = std::max(bit_depth, (size_t)length);
        depth_count.resize(bit_depth);
        depth_count[length - 1]++;
    }
    
    // Now accumulate depths
    for (size_t i = bit_depth - 1; i != 0ULL; i--) {
        depth_count[i - 1] += depth_count[i];
    }

    ASSERT_EQ(wm_huffman_height(t), bit_depth);
    for (size_t j = 0; j < bit_depth; j++) {
        ASSERT_EQ(wm_huffman_size(t, j), depth_count[j]) << "level " << j;
    }
    
    for (size_t i = start; i < end; i++) {
        auto bv = wm_huffman_get(t, i - start);

        auto [ encoded_code, encoded_len ] = codes.encode(expected[i]);
        auto encoded_shifted = encoded_code >> (MAX_HUFFMAN_LENGTH - encoded_len);

        ASSERT_EQ(encoded_shifted, bit_vector_to<size_t>(bv)) << fail_info(expected, codes, t, i, start);
        ASSERT_EQ(encoded_len, bv.size()) << fail_info(expected, codes, t, i, start);

        auto decoded = huffman_codes_decode<value_type>(codes, bv);
        ASSERT_EQ(expected[i], decoded) << fail_info(expected, codes, t, i, start);
    }
}

template <typename value_type, typename wt_impl_type>
void test_wm_huffman_access(const std::vector<value_type>& expected, const huffman_codes_wm<value_type>& codes) {
    test_wm_huffman_access<value_type, wt_impl_type>(expected, codes, 0, expected.size());
}

TEST(WmHuffmanOperations, Access) {
    {
        // wm_huffman of char string
        std::string str = " !??,. lorem ipsum dolor sit amet !??,.";
        std::vector<char> text(str.begin(), str.end());

        huffman_codes_wm<char> codes(text);

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_basic>(text, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_no_lut<8, 8>>(text, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_no_lut<4, 4, uint16_t>>(text, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_no_lut<4, 8, uint32_t>>(text, codes)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_pext<2, uint64_t>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_pext<4, uint64_t>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_pext<8, uint64_t>>(text, codes))  );

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_shuffle<uint64_t>>(text, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_shuffle<__m128i>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_shuffle<__m256i>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<char, wt_params_shuffle<__m512i>>(text, codes))  );
    }

    {
        // wm_huffman of shorts
        std::vector<uint16_t> values { 0xFFFF, 0, 0x8000, 0x1234, 0x4567, 0x2345, 0xABCD };
        huffman_codes_wm<uint16_t> codes(values);

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_basic>(values, codes)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_no_lut<8, 8>>(values, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_no_lut<4, 4, uint16_t>>(values, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_no_lut<4, 8, uint32_t>>(values, codes)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_pext<2, uint64_t>>(values, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_pext<4, uint64_t>>(values, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_pext<8, uint64_t>>(values, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_pext<16, uint64_t>>(values, codes)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_shuffle<uint64_t>>(values, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_shuffle<__m128i>>(values, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_shuffle<__m256i>>(values, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_shuffle<__m512i>>(values, codes))  );
    }
}

TEST(WmHuffmanOperations, Random) {
    {
        random_number_generator<> rng;
        std::vector<uint8_t> text;
        for (size_t i = 0; i < RANDOM_SIZE_A; i++) {
            text.push_back(rng.next());
        }
        huffman_codes_wm<uint8_t> codes(text);

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_basic>(text, codes)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_no_lut<8, 8>>(text, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_no_lut<4, 4, uint16_t>>(text, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_no_lut<4, 8, uint32_t>>(text, codes)) );
 
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_pext<2, uint64_t>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_pext<4, uint64_t>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_pext<8, uint64_t>>(text, codes))  );
 
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_shuffle<uint64_t>>(text, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_shuffle<__m128i>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_shuffle<__m256i>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint8_t, wt_params_shuffle<__m512i>>(text, codes))  );
    }

    {
        random_number_generator<> rng;
        std::vector<uint16_t> text;
        for (size_t i = 0; i < RANDOM_SIZE_B; i++) {
            text.push_back(rng.next() & mask(12));
        }
        huffman_codes_wm<uint16_t> codes(text);

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_basic>(text, codes)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_no_lut<8, 8>>(text, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_no_lut<4, 4, uint16_t>>(text, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_no_lut<4, 8, uint32_t>>(text, codes)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_pext<2, uint64_t>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_pext<4, uint64_t>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_pext<8, uint64_t>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_pext<16, uint64_t>>(text, codes)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_shuffle<uint64_t>>(text, codes)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_shuffle<__m128i>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_shuffle<__m256i>>(text, codes))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_huffman_access<uint16_t, wt_params_shuffle<__m512i>>(text, codes))  );
    }
}
