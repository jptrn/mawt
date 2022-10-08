#include <gtest/gtest.h>

#include <util/bits.hpp>
#include <vector/bit_vector_operations.hpp>
#include <wt/wm.hpp>
#include <wt/wm_operations.hpp>
#include <wt/wt_params.hpp>

#include <util/debug.hpp>
#include <util/random.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <immintrin.h>

#define RANDOM_SIZE_A 512
#define RANDOM_SIZE_B 256

template <typename value_type, typename wt_impl_type>
void test_wm_access(const std::vector<value_type>& expected, size_t start, size_t end, size_t bit_depth) {
    auto t = wm::make<value_type, wt_impl_type>(expected, start, end, bit_depth);
    size_t size = end - start;
    ASSERT_EQ(wm_size(t), size);
    ASSERT_EQ(wm_height(t), bit_depth);

    for (size_t i = start; i < end; i++) {
        auto actual = bit_vector_to<value_type>(wm_get(t, i - start));
        ASSERT_EQ(expected[i] & mask(bit_depth), actual) << "mismatch @ " << i << " [" << (i - start) << " in wm]";
    }
}

template <typename value_type, typename wt_impl_type>
void test_wm_access(const std::vector<value_type>& expected, size_t bit_depth) {
    test_wm_access<value_type, wt_impl_type>(expected, 0, expected.size(), bit_depth);
}

TEST(WmOperations, Access) {
    {
        // Empty WM
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_basic>(std::vector<char>(), 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_lut<4, 4, 1, uint16_t>>(std::vector<char>(), 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_shuffle<uint64_t>>(std::vector<char>(), 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_shuffle<__m128i>>(std::vector<char>(), 8)) );
    }

    {
        // wm of char string
        std::string str = " !??,. lorem ipsum dolor sit amet !??,.";
        std::vector<char> text(str.begin(), str.end());
        
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_basic>(text, 8)) );

        // Must wrap function call in braces so the ',' in the macro is ignored
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_no_lut<8, 8>>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_no_lut<4, 4, uint16_t>>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_no_lut<4, 8, uint32_t>>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_lut<4, 4, 1, uint16_t>>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_lut<4, 4, 2, uint16_t>>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_lut<4, 8, 2, uint32_t>>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_pext<2, uint64_t>>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_pext<4, uint64_t>>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_pext<8, uint64_t>>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_pext<16, uint64_t>>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_shuffle<uint64_t>>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_shuffle<__m128i>>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_shuffle<__m256i>>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_shuffle<__m512i>>(text, 8))  );
    }

    {
        // wm of shorts
        std::vector<uint16_t> values { 0xFFFF, 0, 0x8000, 0x1234, 0x4567, 0x2345, 0xABCD };

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_basic>(values, 15)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_no_lut<8, 8>>(values, 15)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_no_lut<8, 8>>(values, 15)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_no_lut<4, 4, uint16_t>>(values, 15)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_no_lut<4, 8, uint32_t>>(values, 15)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_lut<4, 4, 1, uint16_t>>(values, 15)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_lut<4, 4, 2, uint16_t>>(values, 15)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_lut<4, 8, 2, uint32_t>>(values, 15)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_pext<2, uint64_t>>(values, 15))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_pext<4, uint64_t>>(values, 15))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_pext<8, uint64_t>>(values, 15))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_pext<16, uint64_t>>(values, 15)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_shuffle<uint64_t>>(values, 15)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_shuffle<__m128i>>(values, 15))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_shuffle<__m256i>>(values, 15))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_shuffle<__m512i>>(values, 15))  );
    }

    {
        // Different heights
        std::vector<uint16_t> values { 0xFFFF, 0, 0x8000, 0x1234, 0x4567, 0x2345, 0xABCD, 0x2500, 0x9999, 0xFFFF, 0xF0F0, 0xCCCC };
        for (size_t height = 1; height < 16; height++) {
            ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint16_t, wt_params_lut<4, 4, 1, uint16_t>>(values, height)) );
        }   
    }
}

TEST(WmOperations, Slice) {
    std::string str = " !??,. lorem ipsum dolor sit amet !??,.";
    std::vector<char> text(str.begin(), str.end());

    ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_basic>(text, 0, text.size() / 2, 8)) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_basic>(text, text.size() / 2, text.size(), 8)) );

    ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_pext<4, uint64_t>>(text, 0, text.size() / 2, 8)) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_access<char, wt_params_pext<4, uint64_t>>(text, text.size() / 2, text.size(), 8)) );
}

TEST(WmOperations, Random) {
    {
        random_number_generator<> rng;
        std::vector<uint8_t> text;
        for (size_t i = 0; i < RANDOM_SIZE_A; i++) {
            text.push_back(rng.next());
        }
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_basic>(text, 8)) );

        // Must wrap function call in braces so the ',' in the macro is ignored
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_no_lut<8, 8>>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_no_lut<8, 8>>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_no_lut<4, 4, uint16_t>>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_no_lut<4, 8, uint32_t>>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_lut<4, 4, 1, uint16_t>>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_lut<4, 4, 2, uint16_t>>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_lut<4, 8, 2, uint32_t>>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_pext<2, uint64_t>>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_pext<4, uint64_t>>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_pext<8, uint64_t>>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_pext<16, uint64_t>>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_shuffle<uint64_t>>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_shuffle<__m128i>>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_shuffle<__m256i>>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint8_t, wt_params_shuffle<__m512i>>(text, 8))  );
    }

    {
        random_number_generator<> rng;
        std::vector<uint32_t> text;
        for (size_t i = 0; i < RANDOM_SIZE_B; i++) {
            text.push_back(rng.next());
        }
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_basic>(text, 18)) );

        // Must wrap function call in braces so the ',' in the macro is ignored
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_no_lut<8, 8>>(text, 18)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_no_lut<8, 8>>(text, 18)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_no_lut<4, 4, uint16_t>>(text, 18)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_no_lut<4, 8, uint32_t>>(text, 18)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_lut<4, 4, 1, uint16_t>>(text, 18)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_lut<4, 4, 2, uint16_t>>(text, 18)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_lut<4, 8, 2, uint32_t>>(text, 18)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_pext<2, uint64_t>>(text, 18))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_pext<4, uint64_t>>(text, 18))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_pext<8, uint64_t>>(text, 18))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_pext<16, uint64_t>>(text, 18)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_shuffle<uint64_t>>(text, 18)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_shuffle<__m128i>>(text, 18))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_shuffle<__m256i>>(text, 18))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_access<uint32_t, wt_params_shuffle<__m512i>>(text, 18))  );
    }
}