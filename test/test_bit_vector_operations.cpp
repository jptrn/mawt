#include <gtest/gtest.h>

#include <vector/bit_vector.hpp>
#include <vector/bit_vector_operations.hpp>

#include <cstddef>
#include <cstdint>
#include <string>

TEST(BitVectorOperations, Rank) {
    bit_vector bv(std::vector<bool>   { 1, 0, 0, 1, 1, 0, 1, 0, 0 });
    std::vector<size_t> rank0_results { 0, 1, 2, 2, 2, 3, 3, 4, 5 };
    std::vector<size_t> rank1_results { 1, 1, 1, 2, 3, 3, 4, 4, 4 };

    for (size_t i = 0; i < rank0_results.size(); i++) {
        ASSERT_EQ(bit_vector_rank0(bv, i), rank0_results[i]);
        ASSERT_EQ(bit_vector_rank1(bv, i), rank1_results[i]);
    }
}

TEST(BitVectorOperations, FromString) {
    {
        std::string str = "01001000";

        auto bv = bit_vector_from_string(str);
        ASSERT_EQ(str.length(), bv.size());
        for (size_t i = 0; i < str.length(); i++) {
            ASSERT_EQ(str[i] == '1' ? 1 : 0, bv[i]) << "mismatch @ " << i;
        }
        
        auto bv_reverse = bit_vector_from_string(str, '1', '0', true);
        ASSERT_EQ(str.length(), bv_reverse.size());
        for (size_t i = 0; i < str.length(); i++) {
            ASSERT_EQ(str[i] == '1' ? 1 : 0, bv_reverse[str.length() - 1 - i]) << "mismatch @ " << i;
        }
    }

    {
        std::string str = "aaabbbba";

        auto bv = bit_vector_from_string(str, 'a', 'b');
        ASSERT_EQ(str.length(), bv.size());
        for (size_t i = 0; i < str.length(); i++) {
            ASSERT_EQ(str[i] == 'a' ? 1 : 0, bv[i]) << "mismatch @ " << i;
        }
        
        auto bv_reverse = bit_vector_from_string(str, 'a', 'b', true);
        ASSERT_EQ(str.length(), bv_reverse.size());
        for (size_t i = 0; i < str.length(); i++) {
            ASSERT_EQ(str[i] == 'a' ? 1 : 0, bv_reverse[str.length() - 1 - i]) << "mismatch @ " << i;
        }
    }

    {
        // Skip spaces
        std::string str = "a_b a b b a";
        std::vector<bool> expected { 1, 0, 1, 0, 0, 1 };

        auto bv = bit_vector_from_string(str, 'a', 'b');
        ASSERT_EQ(expected.size(), bv.size());
        for (size_t i = 0; i < expected.size(); i++) {
            ASSERT_EQ(expected[i], bv[i]) << "mismatch @ " << i;
        }
        
        auto bv_reverse = bit_vector_from_string(str, 'a', 'b', true);
        ASSERT_EQ(expected.size(), bv_reverse.size());
        for (size_t i = 0; i < expected.size(); i++) {
            ASSERT_EQ(expected[i], bv_reverse[expected.size() - 1 - i]) << "mismatch @ " << i;
        }
    }

    {
        // Round-trip
        auto bv_a = bit_vector(std::vector<bool> { 0, 1, 0, 1, 1, 1, 1, 0 });
        auto str = bit_vector_to_string(bv_a);
        auto bv_b = bit_vector_from_string(str);
        ASSERT_EQ(bv_a.size(), bv_b.size());
        for (size_t i = 0; i < bv_a.size(); i++) {
            ASSERT_EQ(bv_a[i], bv_b[i]) << "mismatch @ " << i;
        }
    }
}

TEST(BitVectorOperations, ToString) {
    {
        bit_vector bv(std::vector<bool> { 0, 1, 0, 0, 0, 1, 0, 1, 0 });
        ASSERT_STREQ(bit_vector_to_string(bv).c_str(), "010001010");
        ASSERT_STREQ(bit_vector_to_string(bv, '1', '0', true).c_str(), "010100010");
        ASSERT_STREQ(bit_vector_to_string(bv, 'a', 'b').c_str(), "babbbabab");
        ASSERT_STREQ(bit_vector_to_string(bv, 'a', 'b', true).c_str(), "bababbbab");
    }

    // Round-trip
    {
        std::string str_a = "1010101011111010";
        auto bv = bit_vector_from_string(str_a);
        ASSERT_STREQ(str_a.c_str(), bit_vector_to_string(bv).c_str());
    }
}

TEST(BitVectorOperations, ToValue) {
    {
        // 8 bits, matching
        bit_vector bv(std::vector<bool> { 1, 0, 0, 1, 1, 0, 1, 1 });
        ASSERT_EQ(bit_vector_to<uint8_t>(bv), 0b11011001);
    }

    {   // 16 bits, matching
        bit_vector bv(std::vector<bool> { 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1  });
        ASSERT_EQ(bit_vector_to<uint16_t>(bv), 0b1000101101001111);
    }

    {   // 8 bits, must truncate
        bit_vector bv(std::vector<bool> { 1, 0, 0, 1, 1, 0, 1, 1, 0 });
        ASSERT_EQ(bit_vector_to<uint8_t>(bv), 0b11011001);
    }

    {   // 4 bits, truncates
        bit_vector bv(std::vector<bool> { 1, 0, 1, 1 });
        ASSERT_EQ(bit_vector_to<uint8_t>(bv, 4), 0b00001101);
    }
}
