#pragma once

#include <cstddef>
#include <cstdint>

template <typename packed_value_type = uint64_t>
class packed_data {
private:
    size_t m_left_count;
    size_t m_right_count;
    packed_value_type m_left;
    packed_value_type m_right;
    size_t m_bits;

public:
    packed_data() :
        m_left_count(),
        m_right_count(),
        m_left(),
        m_right(),
        m_bits() {}

    packed_data(size_t left_count, size_t right_count, packed_value_type left, packed_value_type right, size_t bits) :
        m_left_count(left_count),
        m_right_count(right_count),
        m_left(left), 
        m_right(right), 
        m_bits(bits) {}

    size_t left_count() const {
        return m_left_count;
    }

    size_t right_count() const {
        return m_right_count;
    }

    packed_value_type left() const {
        return m_left;
    }

    packed_value_type right() const {
        return m_right;
    }

    size_t bits() const {
        return m_bits;
    }

};