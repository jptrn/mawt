#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vector>

#include <util/bits.hpp>
#include <vector/packed/packed_list.hpp>
#include <vector/packed/packed_list_writer.hpp>

TEST(PackedList, Empty) {
    {
        packed_list<2, uint16_t> p(0);
        ASSERT_EQ(0, p.size());
    }

    {
        packed_list<8, uint16_t> p(0);
        ASSERT_EQ(0, p.size());
    }

    {
        packed_list<8, uint64_t> p(0);
        ASSERT_EQ(0, p.size());
    }
}

template <size_t tau, typename packed_value_type>
void test_basic() {
    packed_list<tau, packed_value_type> p(516);
    packed_list_writer<tau, packed_value_type> pw(p);
    ASSERT_EQ(516, p.size());

    std::vector<packed_value_type> expected(p.size());
    for (size_t i = 0; i < p.size(); i++) {
        expected[i] = ((i % 27) + (i % 4)) & mask(tau);
        pw.write(expected[i]);
    }

    const size_t values_per_word = (sizeof(packed_value_type) * 8) / tau;

    // Aligned reads
    for (size_t i = 0; i < p.size(); i += values_per_word) {
        packed_value_type e = 0;
        for (size_t r = 1; r <= values_per_word; r++) {
            size_t read = r - 1;
            if (i + read < p.size()) {
                e |= expected[i + read] << (read * tau);
                ASSERT_EQ(p.pack_aligned(i, r), e) << "mismatch @ " << i << ", " << r << " values read";
            }
        }
    }   

    // Unaligned reads
    for (size_t i = 0; i < p.size(); i++) {
        packed_value_type e = 0;
        size_t cur_word_pos = i % values_per_word;
        for (size_t r = 1; cur_word_pos + r <= values_per_word; r++) {
            size_t read = r - 1;
            if (i + read < p.size()) {
                e |= expected[i + read] << (read * tau);
                ASSERT_EQ(p.pack(i, r), e) << "mismatch @ " << i << ", " << r << " values read";
            }
        }
    }   
}
TEST(PackedList, Basic) {
    ASSERT_NO_FATAL_FAILURE( (test_basic<2,  uint16_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_basic<4,  uint16_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_basic<8,  uint16_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_basic<16, uint16_t>()) );

    ASSERT_NO_FATAL_FAILURE( (test_basic<2,  uint64_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_basic<4,  uint64_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_basic<8,  uint64_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_basic<16, uint64_t>()) );
}

template <size_t tau, typename packed_value_type>
void test_write_multiple() {
    const size_t values_per_word = (sizeof(packed_value_type) * 8) / tau;
    packed_list<tau, packed_value_type> p(516);
    packed_list_writer<tau, packed_value_type> pw(p);
    ASSERT_EQ(516, p.size());

    // Write 'n' values at a time, where n is taken from a random permutation of [0, values_per_word]
    std::vector<size_t> write_counts(values_per_word + 1);
    std::iota(write_counts.begin(), write_counts.end(), 0);
    std::random_shuffle(write_counts.begin(), write_counts.end());
    size_t shuffle_read_i = 0;

    std::vector<packed_value_type> expected(p.size());
    for (size_t i = 0; i < p.size();) {
        size_t values_to_write = std::min(p.size() - i, write_counts[shuffle_read_i++]);
        shuffle_read_i %= write_counts.size();
        packed_value_type e = 0;
        for (size_t v = 0; v < values_to_write; v++) {
            expected[i + v] = (331 + (i % 31) + (i % 7)) & mask(tau);
            e |= expected[i + v] << (v * tau);
        }
        pw.write(values_to_write, e);
        i += values_to_write;
    }

    for (size_t i = 0; i < p.size(); i++) {
        ASSERT_EQ(p.pack(i, 1), expected[i]) << " mismatch @ " << i << std::endl;
    }
}
TEST(PackedList, WriteMultiple) {
    ASSERT_NO_FATAL_FAILURE( (test_write_multiple<2,  uint16_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_write_multiple<4,  uint16_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_write_multiple<8,  uint16_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_write_multiple<16, uint16_t>()) );

    ASSERT_NO_FATAL_FAILURE( (test_write_multiple<2,  uint64_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_write_multiple<4,  uint64_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_write_multiple<8,  uint64_t>()) );
    ASSERT_NO_FATAL_FAILURE( (test_write_multiple<16, uint64_t>()) );
}