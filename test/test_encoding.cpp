#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <util/random.hpp>
#include <vector/encoded/encoded_vector.hpp>
#include <vector/encoded/encoded_vector_writer.hpp>

TEST(Encoding, Basic) {
    encoded_vector<uint64_t, 8> vec(32);
    ASSERT_EQ(vec.size(), 32);
    ASSERT_EQ(vec.capacity(), 40);  // Add 1 element extra
    ASSERT_FALSE(vec.empty());

    vec.resize(17);
    ASSERT_EQ(vec.size(), 17);
    ASSERT_EQ(vec.capacity(), 40);
    ASSERT_FALSE(vec.empty());

    vec.resize(0);
    ASSERT_EQ(vec.size(), 0);
    ASSERT_EQ(vec.capacity(), 40);
    ASSERT_TRUE(vec.empty());

    vec.resize(32);
    ASSERT_EQ(vec.size(), 32);
    ASSERT_EQ(vec.capacity(), 40);
    ASSERT_FALSE(vec.empty());
}

template <typename encoded_value_type, size_t tau>
void test_random() {
    random_number_generator<> rng;
    size_t size = 516;

    std::vector<encoded_value_type> values(size);
    std::vector<encoded_value_type> lengths(size);

    for (size_t i = 0; i < size; i++) {
        values[i] = rng.next() & mask(tau);
        lengths[i] = rng.next() % tau;
    }

    const size_t N = encoded_vector<encoded_value_type, tau>::N;
    for (size_t write_vals = 1; write_vals <= N; write_vals++) {
        encoded_vector<encoded_value_type, tau> vec(size);
        encoded_vector_writer<encoded_value_type, tau> writer(vec);

        // Fill vector
        for (size_t i = 0; i < size; i += write_vals) {
            size_t count = std::min(size - i, write_vals);
            encoded_value_type v = 0;
            encoded_value_type l = 0;
            for (size_t j = 0; j < count; j++) {
                v |= values[i + j] << (j * tau);
                l |= lengths[i + j] << (j * tau);
            }
            writer.write(count, v, l);
        }

        for (size_t read_vals = 1; read_vals <= N; read_vals++) {
            // Read back values
            for (size_t i = 0; i < size; i += read_vals) {
                size_t count = std::min(N - (i % N), read_vals);
                count = std::min(count, size - i);
                auto [ v, l ] = vec.get(i, count);
                for (size_t j = 0; j < count; j++) {
                    ASSERT_EQ(v & mask(tau), values[i + j])  << "mismatch @ " << (i + j)  << ", tau: " << tau << ", N: " << N << ", writing: " << write_vals << ", reading: " << read_vals;
                    ASSERT_EQ(l & mask(tau), lengths[i + j]) << "mismatch @ " << (i + j) << ", tau: " << tau << ", N: " << N << ", writing: " << write_vals << ", reading: " << read_vals;
                    if constexpr (tau < sizeof(encoded_value_type) * 8) {
                        v >>= tau;
                        l >>= tau;
                    }
                }
            }
        }
    }
}
TEST(Encoding, Random) {
    ASSERT_NO_FATAL_FAILURE( (test_random<uint8_t, 2>()) );
    ASSERT_NO_FATAL_FAILURE( (test_random<uint8_t, 4>()) );
    ASSERT_NO_FATAL_FAILURE( (test_random<uint8_t, 8>()) );
    ASSERT_NO_FATAL_FAILURE( (test_random<uint64_t, 2>()) );
    ASSERT_NO_FATAL_FAILURE( (test_random<uint64_t, 4>()) );
    ASSERT_NO_FATAL_FAILURE( (test_random<uint64_t, 8>()) );
    ASSERT_NO_FATAL_FAILURE( (test_random<uint64_t, 16>()) );
    ASSERT_NO_FATAL_FAILURE( (test_random<uint64_t, 32>()) );
    ASSERT_NO_FATAL_FAILURE( (test_random<uint64_t, 64>()) );
}