#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include <vector/int_buffer.hpp>

TEST(IntBuffer, Basic) {
    int_buffer<uint64_t> a(0);
    ASSERT_EQ(a.size(), 0);

    int_buffer<uint64_t> b(4);
    ASSERT_EQ(b.size(), 4);
    b.set(0, 0);
    b.set(1, 5);
    b.set(2, 7);
    b.set(3, 3);
    ASSERT_EQ(b.get(0), 0);
    ASSERT_EQ(b.get(1), 5);
    ASSERT_EQ(b.get(2), 7);
    ASSERT_EQ(b.get(3), 3);
}

TEST(IntBuffer, Indexer) {
    int_buffer<uint64_t> a(4);
    ASSERT_EQ(a.size(), 4);
    a[0] = 4;
    a[1] = 3;
    a[2] = 6;
    a[3] = 4;
    ASSERT_EQ(a[0], 4);
    ASSERT_EQ(a[1], 3);
    ASSERT_EQ(a[2], 6);
    ASSERT_EQ(a[3], 4);
}

TEST(IntBuffer, Copy) {
    int_buffer<uint64_t> a(4);
    ASSERT_EQ(a.size(), 4);
    a[0] = 4;
    a[1] = 3;
    a[2] = 6;
    a[3] = 4;

    int_buffer<uint64_t> b(a);
    ASSERT_EQ(a.size(), 4);
    ASSERT_EQ(b.size(), a.size());

    ASSERT_EQ(a[0], 4);
    ASSERT_EQ(a[1], 3);
    ASSERT_EQ(a[2], 6);
    ASSERT_EQ(a[3], 4);
    for (size_t i = 0; i < a.size(); i++) {
        ASSERT_EQ(a[i], b[i]);
    }
}

TEST(IntBuffer, Iterator) {
    int_buffer<uint64_t> a(4);
    ASSERT_EQ(a.size(), 4);
    a[0] = 4;
    a[1] = 3;
    a[2] = 6;
    a[3] = 4;

    auto it = a.begin();
    ASSERT_TRUE(it != a.end());
    ASSERT_EQ(*it, 4);

    ++it;
    ASSERT_TRUE(it != a.end());
    ASSERT_EQ(*it, 3);

    ++it;
    ASSERT_TRUE(it != a.end());
    ASSERT_EQ(*it, 6);

    ++it;
    ASSERT_TRUE(it != a.end());
    ASSERT_EQ(*it, 4);

    ++it;
    ASSERT_TRUE(it == a.end());

    std::vector<uint64_t> values { 4, 3, 6, 4 };
    size_t i = 0;
    for (auto s : a) {
        ASSERT_EQ(values[i++], s);
    }
}