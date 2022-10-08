#pragma once

#include <huffman/huffman_encoder.hpp>
#include <huffman/huffman_code.hpp>
#include <huffman/huffman_codes.hpp>
#include <util/bits.hpp>
#include <vector/encoded/encoded_vector.hpp>
#include <vector/encoded/encoded_vector_writer.hpp>
#include <wt/encoded/encoded_vector_processor.hpp>
#include <wt/wt_params.hpp>

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <vector>

#include <util/debug.hpp>

// Huffman encoded wavelet matrix

class wm_huffman {
private:
    template <typename wt_impl_type>
    using packed_enc_vec_type = encoded_vector<typename wt_impl_type::packed_value_type, wt_impl_type::tau>;

    template <typename wt_impl_type>
    using packed_enc_vec_writer_type = encoded_vector_writer<typename wt_impl_type::packed_value_type, wt_impl_type::tau>;

    static inline bool in_next(size_t code_length, size_t level) {
        return (code_length - 1) > level;
    }

    std::vector<bit_vector> m_levels;

    template <typename value_type>
    inline void _process_level_basic(
        const std::vector<value_type>& s_cur,
        std::vector<value_type>& s_next,
        const huffman_codes_wm<value_type>& codes,
        size_t& size) {

        size_t level = m_levels.size();
        m_levels.emplace_back(size);

        size_t zero_count = 0;
        size_t one_count = 0;
        for (size_t i = 0; i < size; i++) {
            auto [ code, length ] = codes.encode(s_cur[i]);
            if (in_next(length, level)) {
                auto bit = code & (1ULL << (MAX_HUFFMAN_LENGTH - level - 1)) ? 1 : 0;
                zero_count += !bit;
                one_count += bit;
            }
        }

        size = zero_count + one_count;
        s_next.resize(size);

        size_t zero_i = 0;
        size_t one_i = zero_count;

        for (size_t i = 0; i < s_cur.size(); i++) {
            auto ch = s_cur[i];
            auto [ code, length ] = codes.encode(ch);
            auto bit = code & (1ULL << (MAX_HUFFMAN_LENGTH - level - 1));
            m_levels[level][i] = bit;
            if (in_next(length, level)) {
                s_next[(!bit ? zero_i : one_i)++] = ch;
            }
        }
    }


    template <typename value_type, typename wt_impl_type>
    inline void _make_basic(const std::vector<value_type>& s, const huffman_codes_wm<value_type>& codes, size_t start, size_t end) {
        size_t size = end - start;

        // Prepare buffers for the current and next level for S...
        std::vector<std::vector<value_type>> s_buf;
        s_buf.emplace_back(s.begin() + start, s.begin() + end);
        s_buf.emplace_back(size);

        for (size_t level = 0; size > 0; level++) {
            size_t read = level & 1;
            size_t write = read ^ 1;

            _process_level_basic(s_buf[read], s_buf[write], codes, size);
        }
    }

    template <typename wt_impl_type>
    inline void _process_level_packed(
        const packed_enc_vec_type<wt_impl_type>& p_cur,
        packed_enc_vec_type<wt_impl_type>& p_next,
        size_t& size) {

        constexpr size_t tau = wt_impl_type::tau;
        constexpr size_t N = wt_impl_type::N;

        size_t level = m_levels.size();
        size_t alpha = level % tau;
        m_levels.emplace_back(size);

        size_t zero_count = 0;
        size_t one_count = 0;
        for (size_t i = 0; i < size; i += N) {
            size_t count = std::min(size - i, N);
            auto [ code, length ] = p_cur.get(i, count);
            auto [ zeros_cur, ones_cur ] = encoded_vector_processor<wt_impl_type>::count_bits(code, length, alpha, count);
            zero_count += zeros_cur;
            one_count += ones_cur;
        }

        size = zero_count + one_count;
        p_next.resize(size);

        packed_enc_vec_writer_type<wt_impl_type> write_l(p_next, 0, zero_count);
        packed_enc_vec_writer_type<wt_impl_type> write_r(p_next, zero_count, size);

        for (size_t i = 0; i < p_cur.size(); i += N) {
            size_t count = std::min(p_cur.size() - i, N);
            auto [ code, length ] = p_cur.get(i, count);
            encoded_vector_processor<wt_impl_type>::process_levelwise(i, code, length, alpha, count, write_l, write_r, m_levels[level]);
        }
    }

    template <typename value_type, typename wt_impl_type>
    inline void _make_twophase(const std::vector<value_type>& s, const huffman_codes_wm<value_type>& codes, size_t start, size_t end) {
        // Merged
        constexpr auto tau = wt_impl_type::tau;
        static_assert(MAX_HUFFMAN_LENGTH % tau == 0);

        size_t size = end - start;

        // Prepare buffers
        std::vector<std::vector<value_type>> s_buf;
        s_buf.emplace_back(s.begin() + start, s.begin() + end);
        s_buf.emplace_back(size);

        // ...shuffling S in "small" nodes
        std::vector<packed_enc_vec_type<wt_impl_type>> p_buf;
        p_buf.reserve(2);
        p_buf.emplace_back(size);
        p_buf.emplace_back(size);

        for (size_t big_level = 0; ; big_level += tau) {
            size_t bits = MAX_HUFFMAN_LENGTH - big_level - tau;
            size_t big_read = (big_level / tau) & 1;
            size_t big_write = big_read ^ 1;
            size_t prev_size = size;

            // Transfer tau bits of the Huffman encoding and the remaining bit length of each code (capped by tau)
            packed_enc_vec_writer_type<wt_impl_type> write_p(p_buf[big_level & 1]);
            for (size_t i = 0; i < size; i++) {
                auto [ code, length ] = codes.encode(s_buf[big_read][i]);
                assert(MAX_HUFFMAN_LENGTH - big_level >= tau);
                write_p.write((code >> bits) & mask(tau), std::min(length - big_level - 1, tau));
            }

            // Process up to tau layers of "Small" Nodes
            for (size_t alpha = 0; alpha < tau; alpha++) {
                size_t level = big_level + alpha;

                size_t small_read = level & 1;
                size_t small_write = small_read ^ 1;

                _process_level_packed<wt_impl_type>(p_buf[small_read], p_buf[small_write], size); 
                if (size == 0) {
                    return;
                }
            }

            std::vector<size_t> counts(1ULL << tau);
            for (size_t i = 0; i < prev_size; i++) {
                auto [ code, length ] = codes.encode(s_buf[big_read][i]);
                if ((size_t)length > big_level + tau) {
                    size_t cur = (code >> bits) & mask(tau);
                    counts[bit_reverse_max<tau>(cur)]++;
                }
            }

            std::vector<size_t> writers(1ULL << tau);
            size_t start = 0;
            for (size_t i = 0; i < (1ULL << tau); i++) {
                writers[i] = start;
                start += counts[i];
            }
            s_buf[big_write].resize(size);
            
            for (size_t i = 0; i < prev_size; i++) {
                auto ch = s_buf[big_read][i];
                auto [ code, length ] = codes.encode(ch);
                if ((size_t)length > big_level + tau) {
                    size_t s_bits = (code >> bits) & mask(tau);
                    s_buf[big_write][writers[bit_reverse_max<tau>(s_bits)]++] = ch;
                }
            }
        }
    }

public:
    inline wm_huffman() {}

    template <typename value_type, typename wt_impl_type>
    inline void make_inplace(const std::vector<value_type>& s, const huffman_codes_wm<value_type>& codes, size_t start, size_t end) {
        if constexpr (wt_impl_type::algorithm == wt_algorithm::BASIC) {
            _make_basic<value_type, wt_impl_type>(s, codes, start, end);
        } else {
            _make_twophase<value_type, wt_impl_type>(s, codes, start, end);
        }
    }

    template <typename value_type, typename wt_impl_type>
    static inline wm_huffman make(const std::vector<value_type>& s, const huffman_codes_wm<value_type>& codes, size_t start, size_t end) {
        wm_huffman t;
        t.make_inplace<value_type, wt_impl_type>(s, codes, start, end);
        return t;
    }

    template <typename value_type, typename wt_impl_type>
    static inline wm_huffman make(const std::vector<value_type>& s, const huffman_codes_wm<value_type>& codes) {
        return make<value_type, wt_impl_type>(s, codes, 0, s.size());
    }

    // -- Member Access --
    inline const bit_vector& level(size_t i) const {
        return m_levels[i];
    }

    inline bit_vector& level(size_t i) {
        return m_levels[i];
    }

    inline size_t height() const {
        return m_levels.size();
    }

    inline size_t size(size_t level = 0) const {
        return m_levels.empty() ? 0 : m_levels[level].size();
    }
};
