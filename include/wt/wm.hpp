#pragma once

#include <vector/int_buffer.hpp>
#include <vector/packed/packed_list.hpp>
#include <vector/packed/packed_list_writer.hpp>
#include <wt/packed_list_processor.hpp>
#include <wt/wt_params.hpp>

#include <algorithm>
#include <cstddef>
#include <vector>

#include <util/debug.hpp>

// Wavelet Matrix

class wm {
private:
    template <typename wt_impl_type>
    using packed_list_type = packed_list<wt_impl_type::tau, typename wt_impl_type::packed_value_type>;

    template <typename wt_impl_type>
    using packed_list_writer_type = packed_list_writer<wt_impl_type::tau, typename wt_impl_type::packed_value_type>;

    std::vector<bit_vector> m_levels;

    template <typename value_type>
    inline void _process_level_basic_final(const std::vector<value_type>& s_cur) {
        // In the final level, extract the bits, but do not produce borders for the next level
        // In fact, we wouldn't even need to calculate the borders for the last level
        size_t level = height() - 1;
        for (size_t i = 0; i < s_cur.size(); i++) {
            m_levels[level][i] = s_cur[i] & 1;
        }
    }

    template <typename value_type>
    inline void _process_level_basic(
        const std::vector<value_type>& s_cur,
        std::vector<value_type>& s_next,
        size_t level) {

        size_t max_depth = height();

        // First, get zeros in the current level
        size_t z_next = 0;
        for (size_t i = 0; i < size(); i++) {
            auto bit = s_cur[i] & (1ULL << (max_depth - level - 1));
            z_next += !bit;
        }

        // Now, process zeros
        size_t zero_i = 0;
        size_t one_i = z_next;
        for (size_t i = 0; i < size(); i++) {
            auto c = s_cur[i];
            auto bit = c & (1ULL << (max_depth - level - 1));
            m_levels[level][i] = bit;
            if (!bit) {
                s_next[zero_i++] = c;
            } else {
                s_next[one_i++] = c;
            }
        }
    }

    template <typename value_type, typename wt_impl_type>
    inline void _make_basic(const std::vector<value_type>& s, size_t start, size_t end) {
        size_t max_depth = height();

        // Prepare buffers for the current and next level for S...
        std::vector<std::vector<value_type>> s_buf;
        s_buf.emplace_back(s.begin() + start, s.begin() + end);
        s_buf.emplace_back(size());

        for (size_t level = 0; level < max_depth - 1; level++) {
            size_t read = level & 1;
            size_t write = read ^ 1;
            _process_level_basic(s_buf[read], s_buf[write], level);
        }

        // Only need to copy bits in the final level
        _process_level_basic_final(s_buf[(max_depth - 1) & 1]);
    }

    template <typename wt_impl_type>
    inline void _process_level_packed(
        const packed_list_type<wt_impl_type>& p_cur,
        packed_list_type<wt_impl_type>& p_next,
        size_t level) {

        size_t tau = wt_impl_type::tau;
        size_t N = wt_impl_type::N;
        
        size_t alpha = level % tau; 

        // First, get zeros in the current level
        size_t z_next = 0;
        for (size_t i = 0; i < size(); i += N) {
            size_t count = std::min(size() - i, N);
            auto v = p_cur.pack_aligned(i, count);
            z_next += packed_list_processor<wt_impl_type>::count_zeros(v, alpha, count);
        }

        packed_list_writer_type<wt_impl_type> write_l(p_next, 0, z_next);
        packed_list_writer_type<wt_impl_type> write_r(p_next, z_next, size());

        for (size_t i = 0; i < size(); i += N) {
            size_t count = std::min(size() - i, N);
            auto v = p_cur.pack_aligned(i, count);
            packed_list_processor<wt_impl_type>::process_levelwise(i, v, alpha, count, write_l, write_r, m_levels[level]);
        }
    }

    template <typename wt_impl_type>
    inline void _process_level_packed_final(const packed_list_type<wt_impl_type>& p) {
        // As in the basic case, we only need to copy over the bits in the final level, although we are dealing with packed
        // lists here. We therefore also process the vector in chunks of N
        constexpr auto tau = wt_impl_type::tau;
        constexpr auto N = wt_impl_type::N;
        
        size_t level = height() - 1;
        size_t alpha = level % tau;
        size_t zero_count = 0;

        for (size_t i = 0; i < p.size(); i += N) {
            size_t count = std::min(p.size() - i, N);
            auto v = p.pack_aligned(i, count);
            zero_count += packed_list_processor<wt_impl_type>::count_zeros(v, alpha, count);
            packed_list_processor<wt_impl_type>::write_bits(i, v, alpha, count, m_levels[level]);
        }
    }

    template <typename value_type, typename wt_impl_type>
    inline void _make_twophase(const std::vector<value_type>& s, size_t start, size_t end) {
        constexpr auto tau = wt_impl_type::tau;

        size_t max_depth = height();

        // Prepare buffers for current and next level for...
        // ...shuffling S in "big" nodes
        std::vector<std::vector<value_type>> s_buf;
        s_buf.reserve(2);
        s_buf.emplace_back(s.begin() + start, s.begin() + end);
        s_buf.emplace_back(size());

        // ...shuffling S in "small" nodes
        std::vector<packed_list_type<wt_impl_type>> p_buf;
        p_buf.reserve(2);
        p_buf.emplace_back(size());
        p_buf.emplace_back(size());

        // Process layer of "Big" Nodes
        for (size_t big_level = 0; big_level < max_depth; big_level += tau) {
            size_t bits = max_depth - big_level - tau;
            size_t big_read = (big_level / tau) & 1;
            size_t big_write = big_read ^ 1;

            // Optimization: If this is the final level, just short-circuit and write the bits directly
            if (big_level == max_depth - 1) {
                _process_level_basic_final<value_type>(s_buf[big_read]);
                return;
            }

            // Transfer the current tau bits of each symbol into the packed list - can do this across the entire level
            // If there are less than tau bits left, we need to align each symbol with the MSB of the tau-bit word
            packed_list_writer_type<wt_impl_type> write_p(p_buf[big_level & 1]);
            for (size_t i = 0; i < size(); i++) {
                auto remaining_levels = max_depth - big_level;
                auto word = remaining_levels < tau ? 
                    s_buf[big_read][i] << (tau - remaining_levels) :
                    s_buf[big_read][i] >> bits;
                write_p.write(word & mask(tau));
            }

            // Process tau layers of "Small" Nodes
            for (size_t alpha = 0; alpha < tau; alpha++) {
                size_t level = big_level + alpha;

                size_t small_read = level & 1;
                size_t small_write = small_read ^ 1;

                if (level < max_depth - 1) {
                    _process_level_packed<wt_impl_type>(p_buf[small_read], p_buf[small_write], level); 
                } else {
                    // If this is the final level, we do not need to worry about calculating the borders for the next array - so we just copy the bits and escape
                    _process_level_packed_final<wt_impl_type>(p_buf[small_read]);
                    return;
                }
            }

            // Unlike before, where we could conveniently use the borders from the previous small level
            // we need to do a counting sort pass, sorting the elements by the reverse of their prefix
            // although we can probably go about this in a smarter way
            std::vector<size_t> counts(1ULL << tau);
            for (size_t i = 0; i < size(); i++) {
                size_t cur = (s_buf[big_read][i]) >> bits & mask(tau);
                counts[bit_reverse_max<tau>(cur)]++;
            }
            std::vector<size_t> indices(1ULL << tau);
            for (size_t i = 0; i < indices.size() - 1; i++) {
                indices[i + 1] = indices[i] + counts[i];
            }
            for (size_t i = 0; i < size(); i++) {
                size_t cur = (s_buf[big_read][i]) >> bits & mask(tau);
                size_t& next_write = indices[bit_reverse_max<tau>(cur)];
                s_buf[big_write][next_write++] = s_buf[big_read][i];
            }
        }
    }

public:
    inline wm(size_t size, size_t depth) {
        for (size_t i = 0; i < depth; i++) {
            m_levels.emplace_back(size);
        }
    }

    template <typename value_type, typename wt_impl_type>
    inline void make_inplace(const std::vector<value_type>& s, size_t start, size_t end) {
        if constexpr (wt_impl_type::algorithm == wt_algorithm::BASIC) {
            _make_basic<value_type, wt_impl_type>(s, start, end);
        } else {
            _make_twophase<value_type, wt_impl_type>(s, start, end);
        }
    }

    template <typename value_type, typename wt_impl_type>
    static inline wm make(const std::vector<value_type>& s, size_t start, size_t end, size_t max_depth) {
        wm m(end - start, max_depth);
        m.make_inplace<value_type, wt_impl_type>(s, start, end);
        return m;
    }

    template <typename value_type , typename wt_impl_type>
    static inline wm make(const std::vector<value_type>& s, size_t max_depth) {
        return make<value_type, wt_impl_type>(s, 0, s.size(), max_depth);
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

    inline size_t size() const {
        return m_levels.empty() ? 0 : m_levels[0].size();
    }
};