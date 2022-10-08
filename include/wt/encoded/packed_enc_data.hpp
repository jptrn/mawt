#pragma once

#include <cstddef>

template <typename packed_enc_value_type>
class packed_enc_data {
private:
    size_t m_left_count;
    size_t m_right_count;
    packed_enc_value_type m_left;
    packed_enc_value_type m_right;
    packed_enc_value_type m_left_lengths;
    packed_enc_value_type m_right_lengths;

public:
    packed_enc_data() :
        m_left_count(),
        m_right_count(),
        m_left(),
        m_right(),
        m_left_lengths(),
        m_right_lengths() {}

    packed_enc_data(
        size_t left_count, 
        size_t right_count, 
        packed_enc_value_type left, 
        packed_enc_value_type right, 
        packed_enc_value_type left_lengths, 
        packed_enc_value_type right_lengths) :
        
        m_left_count(left_count),
        m_right_count(right_count),
        m_left(left), 
        m_right(right), 
        m_left_lengths(left_lengths), 
        m_right_lengths(right_lengths) {}

    size_t left_count() const {
        return m_left_count;
    }

    size_t right_count() const {
        return m_right_count;
    }

    packed_enc_value_type left() const {
        return m_left;
    }

    packed_enc_value_type right() const {
        return m_right;
    }

    packed_enc_value_type left_lengths() const {
        return m_left_lengths;
    }

    packed_enc_value_type right_lengths() const {
        return m_right_lengths;
    }
};