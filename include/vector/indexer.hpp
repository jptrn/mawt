#pragma once

#include <cstddef>

template <typename container_type, typename value_type>
class indexer {
private:
    container_type& m_vector;
    size_t m_index;

public:
    inline indexer(container_type& v, size_t index) : m_vector(v), m_index(index) {}

    inline indexer(const container_type& v, size_t index) : indexer(const_cast<container_type&>(v), index) {}

    inline value_type operator=(value_type value) {
        m_vector.set(m_index, value);
        return value;
    }

    inline value_type operator|=(value_type value) {
        m_vector.set(m_index, m_vector.get(m_index) | value);
        return m_vector.get(m_index);
    }

    inline value_type operator&=(value_type value) {
        m_vector.set(m_index, m_vector.get(m_index) & value);
        return m_vector.get(m_index);
    }

    inline operator value_type() const {
        return m_vector.get(m_index);
    }  
};