#include <gtest/gtest.h>

#include <vector/bit_vector.hpp>
#include <util/bits.hpp>
#include <util/random.hpp>

#include <cstddef>
#include <vector>

TEST(BitVector, Basic) {
    bit_vector empty;
    ASSERT_EQ(empty.size(), 0);

    std::vector<size_t> sizes { 0, 1, 65, 128 };
    for (auto size : sizes) {
        bit_vector b(size);
        b.clear();
        ASSERT_EQ(b.size(), size);
        for (size_t i = 0; i < size; i++) {
            ASSERT_EQ(b[i], 0);
        }
        for (size_t i = 0; i < size; i++) {
            b[i] = i % 3;
            ASSERT_EQ(b[i], (bool)(i % 3));
        }
    }
}

template <size_t t>
void test_set(size_t size = 48) {
    random_number_generator rng;
    std::vector<size_t> values;
    for (size_t i = 0; i < size; i++) {
        values.push_back(rng.next() & mask(t));
    }
    bit_vector b(size * t);
    for (size_t i = 0; i < size; i++) {
        b.set<t>(i * t, values[i]);
    }
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < t; j++) {
            auto expected = (bool)(values[i] & (1ULL << j));
            auto actual = b[i * t + j];
            ASSERT_EQ(expected, actual) << " mismatch @ " << i << ", bit " << j << ", bitcount: " << t;
        }
    }
}
TEST(BitVector, Set) {
    ASSERT_NO_FATAL_FAILURE(test_set<3>());
    ASSERT_NO_FATAL_FAILURE(test_set<8>());
    ASSERT_NO_FATAL_FAILURE(test_set<16>());
}

TEST(BitVector, Constructor) {
    random_number_generator<> rng;
    const size_t size = 1025;
    std::vector<bool> bits(size);
    for (size_t i = 0; i < size; i++) {
        bits[i] = rng.next() & 1;
    }     
    bit_vector b(bits);
    ASSERT_EQ(bits.size(), size);
    for (size_t i = 0; i < size; i++) {
        ASSERT_EQ(b[i], bits[i]) << "Bit Mismatch @ " << i << ", expected: " << bits[i] << ", actual: " << b[i] << ", rng seed: " << rng.seed();
    }
}
