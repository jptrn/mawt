#pragma once

#include <cstddef>

struct huffman_tree {
private:
    size_t m_code;
    size_t m_freq;
    huffman_tree* m_left;
    huffman_tree* m_right;

public:
    inline huffman_tree(size_t code, size_t freq) :
        m_code(code),
        m_freq(freq),
        m_left(nullptr),
        m_right(nullptr) {}

    inline huffman_tree(huffman_tree* l, huffman_tree* r) :
        m_code(-1ULL),
        m_freq(l->m_freq + r->m_freq),
        m_left(l),
        m_right(r) {}

    inline bool is_leaf() const {
        return !m_left && !m_right;
    }

    // RO3

    inline ~huffman_tree() {
        delete m_left;
        delete m_right;
    }

    inline huffman_tree (const huffman_tree& other) : 
        m_code(other.m_code),
        m_freq(other.m_freq),
        m_left(other.m_left),
        m_right(other.m_right) {}

    inline huffman_tree& operator=(const huffman_tree& other) {
        return *this = huffman_tree(other);
    }

    inline size_t freq() const {
        return m_freq;
    }

    inline size_t code() const {
        return m_code;
    }

    inline huffman_tree* left() {
        return m_left;
    }

    inline const huffman_tree* left() const {
        return m_left;
    }

    inline huffman_tree* right() {
        return m_right;
    }

    inline const huffman_tree* right() const {
        return m_right;
    }
};