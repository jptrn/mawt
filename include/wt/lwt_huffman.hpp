#pragma once

#include <huffman/huffman_encoder.hpp>
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

// The Levelwise Wavelet Tree, Huffman Encoded

class lwt_huffman {
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
        const std::vector<size_t>& borders_cur,
        std::vector<size_t>& borders_next,
        const huffman_codes_lwt<value_type>& codes,
        size_t& size) {

        size_t level = m_levels.size();
        m_levels.emplace_back(size);
        
        size = 0;
        borders_next.resize(borders_cur.size() * 2);
        size_t intervals_count = 1ULL << level;

        // Process level in two passes
        // First, calculate the borders for the next level...
        size_t start = 0;
        for (size_t k = 0; k < intervals_count; k++) {
            size_t zero_count = 0;
            size_t next_level_count = 0;

            if (start == m_levels[level].size()) {
                // Limit number of intervals
                intervals_count = k;
                borders_next.resize(intervals_count * 2);
                break;
            }

            for (size_t j = start; j < borders_cur[k]; j++) {
                auto [ code, length ] = codes.encode(s_cur[j]);
                auto bit = code & (1ULL << (MAX_HUFFMAN_LENGTH - level - 1));
                zero_count += in_next(length, level) && !bit;
                next_level_count += in_next(length, level); // Calculate size of next level
            }

            start = borders_cur[k];
            borders_next[k * 2] = size + zero_count;  
            borders_next[k * 2 + 1] = size + next_level_count;
            size = borders_next[k * 2 + 1];
        }

        s_next.resize(size);

        // ...and write the values and shuffle the subsequences for the next level
        size_t i = 0;
        for (size_t k = 0; k < intervals_count; k++) {
            size_t zero_i = k == 0 ? 0 : borders_next[k * 2 - 1];
            size_t one_i = borders_next[k * 2];
            while (i < borders_cur[k]) {
                auto ch = s_cur[i];
                auto [ code, length ] = codes.encode(ch);
                auto bit = code & (1ULL << (MAX_HUFFMAN_LENGTH - level - 1));
                m_levels[level][i] = bit;
                if (in_next(length, level)) {
                    s_next[(!bit ? zero_i : one_i)++] = ch;
                }
                i++;
            }
        }
    }


    template <typename value_type, typename wt_impl_type>
    inline void _make_basic(const std::vector<value_type>& s, const huffman_codes_lwt<value_type>& codes, size_t start, size_t end) {
        size_t size = end - start;

        // Prepare buffers for the current and next level for S...
        std::vector<std::vector<value_type>> s_buf;
        s_buf.emplace_back(s.begin() + start, s.begin() + end);
        s_buf.emplace_back(size);

        // ...and the borders
        std::vector<std::vector<size_t>> borders_buf(2);
        borders_buf[0].push_back(size);

        for (size_t level = 0; size > 0; level++) {
            size_t read = level & 1;
            size_t write = read ^ 1;
            _process_level_basic(s_buf[read], s_buf[write], borders_buf[read], borders_buf[write], codes, size);
        }
    }

    template <typename wt_impl_type>
    inline void _process_level_packed(
        const packed_enc_vec_type<wt_impl_type>& p_cur,
        packed_enc_vec_type<wt_impl_type>& p_next,
        const std::vector<size_t>& borders_cur,
        std::vector<size_t>& borders_next,
        size_t& size) {

        constexpr size_t tau = wt_impl_type::tau;
        constexpr size_t N = wt_impl_type::N;

        size_t level = m_levels.size();
        size_t alpha = level % tau;
        m_levels.emplace_back(size);

        size = 0;
        borders_next.resize(borders_cur.size() * 2);
        size_t intervals_count = 1ULL << level;

        // Process level in two passes
        // First, calculate the borders for the next level...
        size_t start = 0;
        for (size_t k = 0; k < intervals_count; k++) {
            size_t end = borders_cur[k];
            size_t zero_count = 0;
            size_t one_count = 0;

            if (start == m_levels[level].size()) {
                intervals_count = k;
                borders_next.resize(intervals_count * 2);
                break;
            }

            // Align with the next packed list boundary
            size_t align_count = std::min(end, divceil(start, N) * N) - start;
            if (align_count != 0) {
                auto [ code, length ] = p_cur.get(start, align_count);
                auto [ zeros_cur, ones_cur ] = encoded_vector_processor<wt_impl_type>::count_bits(code, length, alpha, align_count);
                zero_count += zeros_cur;
                one_count += ones_cur;
                assert(zeros_cur + ones_cur <= align_count);
            }

            for (size_t j = start + align_count; j < end; j += N) {
                assert(j % N == 0);
                size_t count = std::min(end - j, N);
                auto [ code, length ] = p_cur.get(j, count);
                auto [ zeros_cur, ones_cur ] = encoded_vector_processor<wt_impl_type>::count_bits(code, length, alpha, count);
                zero_count += zeros_cur;
                one_count += ones_cur;
                assert(zeros_cur + ones_cur <= N);
            }

            assert(zero_count + one_count <= end - start);
            start = end;
            borders_next[k * 2] = size + zero_count;  
            borders_next[k * 2 + 1] = borders_next[k * 2] + one_count;
            size = borders_next[k * 2 + 1];
        }

        p_next.resize(size);

        // ...and write the values and shuffle the subsequences for the next level
        size_t i = 0;
        for (size_t k = 0; k < intervals_count; k++) {
            size_t end = borders_cur[k];
            packed_enc_vec_writer_type<wt_impl_type> write_l(p_next, k == 0 ? 0 : borders_next[k * 2 - 1], borders_next[k * 2]);
            packed_enc_vec_writer_type<wt_impl_type> write_r(p_next, borders_next[k * 2], borders_next[k * 2 + 1]);

            // Align ourselves with the next boundary
            size_t count = std::min(end, divceil(i, N) * N) - i;
            if (count != 0) {
                auto [ code, length ] = p_cur.get(i, count);
                encoded_vector_processor<wt_impl_type>::process_levelwise(i, code, length, alpha, count, write_l, write_r, m_levels[level]);
                i += count;
            }
            
            while (i < end) {
                assert(i % N == 0);
                size_t count = std::min(end - i, N);
                auto [ code, length ] = p_cur.get(i, count);
                encoded_vector_processor<wt_impl_type>::process_levelwise(i, code, length, alpha, count, write_l, write_r, m_levels[level]);
                i += count;
            }
        }
    }

    template <typename value_type, typename wt_impl_type>
    inline void _make_twophase(const std::vector<value_type>& s, const huffman_codes_lwt<value_type>& codes, size_t start, size_t end) {
        constexpr auto tau = wt_impl_type::tau;
        static_assert(MAX_HUFFMAN_LENGTH % tau == 0);

        size_t size = end - start;

        // Prepare buffers
        std::vector<std::vector<value_type>> s_buf;
        s_buf.emplace_back(s.begin() + start, s.begin() + end);
        s_buf.emplace_back(size);

        // ...maintaining the borders
        std::vector<std::vector<size_t>> b_buf(2);
        b_buf[0].push_back(size);

        // ...shuffling S in "small" nodes
        std::vector<packed_enc_vec_type<wt_impl_type>> p_buf;
        p_buf.reserve(2);
        p_buf.emplace_back(size);
        p_buf.emplace_back(size);

        for (size_t big_level = 0; ; big_level += tau) {
            size_t bits = MAX_HUFFMAN_LENGTH - big_level - tau;
            size_t big_read = (big_level / tau) & 1;
            size_t big_write = big_read ^ 1;

            // Remember the old borders for the current level since we'll use them to calculate the subsequences for the next big level
            const auto& cur_borders = b_buf[big_level & 1];
            std::vector<size_t> big_borders_prev(cur_borders);

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

                _process_level_packed<wt_impl_type>(p_buf[small_read], p_buf[small_write], b_buf[small_read], b_buf[small_write], size); 
                if (size == 0) {
                    return;
                }
            }

            // Calculate subsequences for next layer of big nodes
            // We already calculated the borders for the layer in the final layer of small nodes
            std::vector<size_t> writers;
            size_t start = 0;
            for (size_t i = 0; i < b_buf[(big_level + tau) & 1].size(); i++) {
                writers.emplace_back(start);
                start = b_buf[(big_level + tau) & 1][i];
            }

            // ...and shuffle S according to its tau current bits
            for (size_t k = 0; k < big_borders_prev.size(); k++) {
                size_t start = k == 0 ? 0 : big_borders_prev[k - 1];
                size_t end = big_borders_prev[k];

                for (size_t j = start; j < end; j++) {
                    size_t ch = s_buf[big_read][j];
                    auto [ code, length ] = codes.encode(ch);
                    if ((size_t)length > big_level + tau) {
                        size_t s_bits = (code >> bits) & mask(tau);
                        size_t next_node = k * (1ULL << tau) + s_bits;
                        s_buf[big_write][writers[next_node]++] = ch;
                    }
                }
            }
        }
    }

public:
    inline lwt_huffman() {}

    template <typename value_type, typename wt_impl_type>
    inline void make_inplace(const std::vector<value_type>& s, const huffman_codes_lwt<value_type>& codes, size_t start, size_t end) {
        if constexpr (wt_impl_type::algorithm == wt_algorithm::BASIC) {
            _make_basic<value_type, wt_impl_type>(s, codes, start, end);
        } else {
            _make_twophase<value_type, wt_impl_type>(s, codes, start, end);
        }
    }

    template <typename value_type, typename wt_impl_type>
    static inline lwt_huffman make(const std::vector<value_type>& s, const huffman_codes_lwt<value_type>& codes, size_t start, size_t end) {
        lwt_huffman t;
        t.make_inplace<value_type, wt_impl_type>(s, codes, start, end);
        return t;
    }

    template <typename value_type, typename wt_impl_type>
    static inline lwt_huffman make(const std::vector<value_type>& s, const huffman_codes_lwt<value_type>& codes) {
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
