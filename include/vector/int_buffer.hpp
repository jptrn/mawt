#pragma once

// A fixed-size array that does not initialize itself 

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <utility>

#include <util/bits.hpp>
#include <vector/indexer.hpp>

#include <util/debug.hpp>


template <typename value_type = uint64_t>
class int_buffer {
private:
    static const size_t ALIGN = 64;
    using int_buffer_indexer = indexer<int_buffer, value_type>;

    value_type* m_data;
    size_t m_size;

public:
    inline int_buffer(size_t size) : m_size(size) {
        size_t byte_size = sizeof(value_type) * size;
        size_t alloc_size = divceil(byte_size, ALIGN) * ALIGN;
        m_data = (value_type*)std::aligned_alloc(ALIGN, alloc_size);
    }

    // -- RO5 --

    inline ~int_buffer() {
        free(m_data);
    }

    inline int_buffer(const int_buffer& other) : m_size(other.m_size) {
        *this = other;
    }

    int_buffer(int_buffer&& other) {
        *this = std::move(other);
    }

    inline int_buffer& operator=(const int_buffer& other) {
        if (this == &other) {
            return *this;
        }

        m_size = other.m_size;

        size_t byte_size = sizeof(value_type) * m_size;
        size_t alloc_size = divceil(byte_size, ALIGN) * ALIGN;
        m_data = (value_type*)std::aligned_alloc(ALIGN, alloc_size);

        for (size_t i = 0; i < m_size; i++) {
            m_data[i] = other.m_data[i];
        }

        return *this;
    }

    inline int_buffer& operator=(int_buffer&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        m_data = std::move(other.m_data);
        m_size = other.m_size;

        other.m_data = nullptr;

        return *this;
    }

    // -- Operations --

    inline void set(size_t i, value_type value) {
        m_data[i] = value;
    }

    inline value_type get(size_t i) const {
        return m_data[i];
    }

    inline value_type* data() {
        return m_data;
    }

    inline const value_type* data() const {
        return m_data;
    }

    inline size_t size() const {
        return m_size;
    }

    inline int_buffer_indexer operator[](size_t i) {
        return int_buffer_indexer(*this, i);
    }

    inline const int_buffer_indexer operator[](size_t i) const {
        return int_buffer_indexer(*this, i);
    }

    // Iterator support
    struct int_buffer_iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using reference         = const value_type&;

    private:
        const int_buffer& m_buf;
        size_t m_pos;

    public:
        int_buffer_iterator(const int_buffer& c, size_t position) : m_buf(c), m_pos(position) {}

        reference operator*() const { 
            return m_buf.m_data[m_pos]; 
        }

        int_buffer_iterator& operator++() { 
            m_pos++; 
            return *this; 
        }  

        int_buffer_iterator operator++(int) { 
            auto tmp = *this; 
            ++(*this); 
            return tmp; 
        }

        friend bool operator==(const int_buffer_iterator& a, const int_buffer_iterator& b) { 
            return a.m_pos == b.m_pos; 
        }

        friend bool operator!=(const int_buffer_iterator& a, const int_buffer_iterator& b) { 
            return a.m_pos != b.m_pos; 
        }     
    };

    inline int_buffer_iterator begin() const { 
        return int_buffer_iterator(*this, 0); 
    }

    inline int_buffer_iterator end() const {
        return int_buffer_iterator(*this, size()); 
    }
};