#include <gtest/gtest.h>

#include <util/bits.hpp>
#include <vector/bit_vector_operations.hpp>
#include <wt/parallel/wm_dd.hpp>
#include <wt/parallel/wt_dd_params.hpp>
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
#include <omp.h>

#define RANDOM_SIZE_A 2048
#define RANDOM_SIZE_B 256
#define CHECK_DENSITY(t, s) ((t) < (s) ? 1 : (s))

template <typename value_type, typename wt_impl_type, typename wt_dd_impl_type>
void test_wm_dd_access(const std::vector<value_type>& expected, size_t bit_depth) {
    omp_set_num_threads(wt_dd_impl_type::num_threads);
    auto t = wm_make_dd<value_type, wt_impl_type, wt_dd_impl_type>(expected, bit_depth);
    ASSERT_EQ(wm_size(t), expected.size());
    ASSERT_EQ(wm_height(t), bit_depth);
    for (size_t i = 0; i < expected.size(); i += CHECK_DENSITY(wm_size(t), 139)) {
        auto actual = bit_vector_to<value_type>(wm_get(t, i));
        ASSERT_EQ(expected[i] & mask(bit_depth), actual) << "mismatch @ " << i;
    }
}

template <typename wt_dd_impl_type>
void test_wm_dd() {
    {
        // Empty wm
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_basic, wt_dd_impl_type>(std::vector<char>(), 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_lut<4, 4, 1, uint16_t>, wt_dd_impl_type>(std::vector<char>(), 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_shuffle<uint64_t>, wt_dd_impl_type>(std::vector<char>(), 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_shuffle<__m128i>, wt_dd_impl_type>(std::vector<char>(), 8)) );
    }

    {
        // wm of char string
        std::string str = " !??,. lorem ipsum dolor sit amet !??,.";
        std::vector<char> text(str.begin(), str.end());
        
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_basic, wt_dd_impl_type>(text, 8)) );

        // Must wrap function call in braces so the ',' in the macro is ignored
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_no_lut<8, 8>, wt_dd_impl_type>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_no_lut<4, 4, uint16_t>, wt_dd_impl_type>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_no_lut<4, 8, uint32_t>, wt_dd_impl_type>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_lut<4, 4, 1, uint16_t>, wt_dd_impl_type>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_lut<4, 4, 2, uint16_t>, wt_dd_impl_type>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_lut<4, 8, 2, uint32_t>, wt_dd_impl_type>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_pext<2, uint64_t>, wt_dd_impl_type>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_pext<4, uint64_t>, wt_dd_impl_type>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_pext<8, uint64_t>, wt_dd_impl_type>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_pext<16, uint64_t>, wt_dd_impl_type>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_shuffle<uint64_t>, wt_dd_impl_type>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_shuffle<__m128i>, wt_dd_impl_type>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_shuffle<__m256i>, wt_dd_impl_type>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<char, wt_params_shuffle<__m512i>, wt_dd_impl_type>(text, 8))  );
    }

    {
        // wm of shorts
        std::vector<uint16_t> values { 0xFFFF, 0, 0x8000, 0x1234, 0x4567, 0x2345, 0xABCD };

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_basic, wt_dd_impl_type>(values, 15)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_no_lut<8, 8>, wt_dd_impl_type>(values, 15)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_no_lut<8, 8>, wt_dd_impl_type>(values, 15)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_no_lut<4, 4, uint16_t>, wt_dd_impl_type>(values, 15)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_no_lut<4, 8, uint32_t>, wt_dd_impl_type>(values, 15)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_lut<4, 4, 1, uint16_t>, wt_dd_impl_type>(values, 15)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_lut<4, 4, 2, uint16_t>, wt_dd_impl_type>(values, 15)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_lut<4, 8, 2, uint32_t>, wt_dd_impl_type>(values, 15)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_pext<2, uint64_t>, wt_dd_impl_type>(values, 15))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_pext<4, uint64_t>, wt_dd_impl_type>(values, 15))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_pext<8, uint64_t>, wt_dd_impl_type>(values, 15))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_pext<16, uint64_t>, wt_dd_impl_type>(values, 15)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_shuffle<uint64_t>, wt_dd_impl_type>(values, 15)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_shuffle<__m128i>, wt_dd_impl_type>(values, 15))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_shuffle<__m256i>, wt_dd_impl_type>(values, 15))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_shuffle<__m512i>, wt_dd_impl_type>(values, 15))  );
    }

    {
        // Different heights
        std::vector<uint16_t> values { 0xFFFF, 0, 0x8000, 0x1234, 0x4567, 0x2345, 0xABCD, 0x2500, 0x9999, 0xFFFF, 0xF0F0, 0xCCCC };
        for (size_t height = 1; height < 16; height++) {
            ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint16_t, wt_params_lut<4, 4, 1, uint16_t>, wt_dd_impl_type>(values, height)) );
        }
    }
}
TEST(WmDdOperations, TestWmDdAccess) {
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd<wt_dd_params<uint64_t, 1>>()) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd<wt_dd_params<uint64_t, 2>>()) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd<wt_dd_params<uint64_t, 4>>()) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd<wt_dd_params<uint64_t, 6>>()) );

    ASSERT_NO_FATAL_FAILURE( (test_wm_dd<wt_dd_params<__m128i, 4>>()) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd<wt_dd_params<__m256i, 4>>()) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd<wt_dd_params<__m512i, 4>>()) );
}

template <typename wt_dd_impl_type>
void test_wm_dd_random() {
    {
        random_number_generator<> rng;
        std::vector<uint8_t> text;
        for (size_t i = 0; i < RANDOM_SIZE_A; i++) {
            text.push_back(rng.next());
        }
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_basic, wt_dd_impl_type>(text, 8)) );

        // Must wrap function call in braces so the ',' in the macro is ignored
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_no_lut<8, 8>, wt_dd_impl_type>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_no_lut<8, 8>, wt_dd_impl_type>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_no_lut<4, 4, uint16_t>, wt_dd_impl_type>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_no_lut<4, 8, uint32_t>, wt_dd_impl_type>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_lut<4, 4, 1, uint16_t>, wt_dd_impl_type>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_lut<4, 4, 2, uint16_t>, wt_dd_impl_type>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_lut<4, 8, 2, uint32_t>, wt_dd_impl_type>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_pext<2, uint64_t>, wt_dd_impl_type>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_pext<4, uint64_t>, wt_dd_impl_type>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_pext<8, uint64_t>, wt_dd_impl_type>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_pext<16, uint64_t>, wt_dd_impl_type>(text, 8)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_shuffle<uint64_t>, wt_dd_impl_type>(text, 8)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_shuffle<__m128i>, wt_dd_impl_type>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_shuffle<__m256i>, wt_dd_impl_type>(text, 8))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint8_t, wt_params_shuffle<__m512i>, wt_dd_impl_type>(text, 8))  );
    }

    {
        random_number_generator<> rng;
        std::vector<uint32_t> text;
        for (size_t i = 0; i < RANDOM_SIZE_B; i++) {
            text.push_back(rng.next());
        }
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_basic, wt_dd_impl_type>(text, 18)) );

        // Must wrap function call in braces so the ',' in the macro is ignored
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_no_lut<8, 8>, wt_dd_impl_type>(text, 18)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_no_lut<8, 8>, wt_dd_impl_type>(text, 18)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_no_lut<4, 4, uint16_t>, wt_dd_impl_type>(text, 18)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_no_lut<4, 8, uint32_t>, wt_dd_impl_type>(text, 18)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_lut<4, 4, 1, uint16_t>, wt_dd_impl_type>(text, 18)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_lut<4, 4, 2, uint16_t>, wt_dd_impl_type>(text, 18)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_lut<4, 8, 2, uint32_t>, wt_dd_impl_type>(text, 18)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_pext<2, uint64_t>, wt_dd_impl_type>(text, 18))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_pext<4, uint64_t>, wt_dd_impl_type>(text, 18))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_pext<8, uint64_t>, wt_dd_impl_type>(text, 18))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_pext<16, uint64_t>, wt_dd_impl_type>(text, 18)) );

        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_shuffle<uint64_t>, wt_dd_impl_type>(text, 18)) );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_shuffle<__m128i>, wt_dd_impl_type>(text, 18))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_shuffle<__m256i>, wt_dd_impl_type>(text, 18))  );
        ASSERT_NO_FATAL_FAILURE( (test_wm_dd_access<uint32_t, wt_params_shuffle<__m512i>, wt_dd_impl_type>(text, 18))  );
    }
}
TEST(WmDdOperations, TestWmDdRandom) {
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd_random<wt_dd_params<uint64_t, 1>>()) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd_random<wt_dd_params<uint64_t, 2>>()) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd_random<wt_dd_params<uint64_t, 4>>()) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd_random<wt_dd_params<uint64_t, 6>>()) );

    ASSERT_NO_FATAL_FAILURE( (test_wm_dd_random<wt_dd_params<__m128i, 4>>()) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd_random<wt_dd_params<__m256i, 4>>()) );
    ASSERT_NO_FATAL_FAILURE( (test_wm_dd_random<wt_dd_params<__m512i, 4>>()) );
}