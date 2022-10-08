#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <utility>

#ifndef NDEBUG
#define NDEBUG
#endif

#include <wx_dd_pc.hpp>
#include <wx_dd_pc_ss.hpp>
#include <wx_dd_ps.hpp>
#include <wx_huff_dd_naive.hpp>
#include <wx_huff_dd_pc.hpp>
#include <wx_huff_dd_ps.hpp>
#include <wx_huff_naive.hpp>
#include <wx_huff_pc.hpp>
#include <wx_huff_pc_ss.hpp>
#include <wx_huff_ps.hpp>
#include <wx_naive.hpp>
#include <wx_pc.hpp>
#include <wx_pc_ss.hpp>
#include <wx_ps.hpp>

#include <util/histogram.hpp>
#include <huffman/huff_codes.hpp>
#include <huffman/huff_level_sizes_builder.hpp>

// Wrapper for Kurpicz's pwm 
// https://github.com/kurpicz/pwm

template <typename wt_value_type, typename wt_pwm_type, size_t v_threads = 1>
class wt_wrapper_pwm {
private:

    static constexpr size_t num_threads = v_threads;
    static constexpr bool is_tree = wt_pwm_type::is_tree;
    static constexpr bool is_huffman = wt_pwm_type::is_huffman_shaped;

    using huffman_codes = canonical_huff_codes<wt_value_type, is_tree>;

    // Used only for decoding Huffman codes
    const std::vector<wt_value_type>& m_text;
    std::unique_ptr<huffman_codes> m_huffman_codes;

    const wavelet_structure m_wx;

    inline bool get(size_t level, size_t i) const {
        return bit_at(m_wx.bvs()[level], i);
    }

    inline size_t rank1(size_t level, size_t start, size_t end) const {
        if (end < start) {
            return 0;
        }
        size_t rank = 0;
        for (size_t i = start; i <= end; i++) {
            rank += get(level, i);
        }
        return rank;
    }

    inline size_t rank1(size_t level, size_t k) const {
        return rank1(level, 0, k);
    }

    inline size_t rank1(size_t level) const {
        return rank1(level, m_wx.bvs().level_bit_size(level) - 1);
    }

    inline size_t rank0(size_t level, size_t start, size_t end) const {
        return (end - start + 1) - rank1(level, start, end);
    }

    inline size_t rank0(size_t level, size_t k) const {
        return rank0(level, 0, k);
    }

    inline size_t rank0(size_t level) const {
        return rank0(level, m_wx.bvs().level_bit_size(level) - 1);
    }

    inline const huffman_codes& get_huffman_codes() const {
        if (!m_huffman_codes) {
            histogram<wt_value_type> hist(m_text.data(), m_text.size());
            level_sizes_builder<wt_value_type> builder(std::move(hist));
            auto nc_this = const_cast<wt_wrapper_pwm*>(this); // Ugly hack, but this is purely about caching
            nc_this->m_huffman_codes = std::make_unique<huffman_codes>(builder);
        }
        return *m_huffman_codes.get();
    }

    inline size_t access_wt(size_t i) const {
        size_t start = 0;
        size_t end = m_wx.bvs().level_bit_size(0) - 1;
        size_t result = 0;
        size_t level = 0;
        for (; level < m_wx.levels() && i < m_wx.bvs().level_bit_size(level); level++) {
            result <<= 1;
            size_t zero_count = rank0(level, start, end);
            if (!get(level, i)) {
                i = start + rank0(level, start, i) - 1;
                end = start + zero_count - 1;
            } else {
                result |= 1;
                i = start + zero_count + rank1(level, start, i) - 1;
                start += zero_count;
            }
        }

        if constexpr (is_huffman) {
            return get_huffman_codes().decode_symbol(level, result);
        } else {
            return result;
        }
    }

    inline size_t access_wm(size_t i) const {
        size_t result = 0;
        size_t level = 0;
        for (; level < m_wx.levels() && i < m_wx.bvs().level_bit_size(level); level++) {
            result <<= 1;
            if (!get(level, i)) {
                i = rank0(level, i) - 1;
            } else {
                result |= 1;
                i = rank0(level) + rank1(level, i) - 1;
            }
        }

        if constexpr (is_huffman) {
            return get_huffman_codes().decode_symbol(level, result);
        } else {
            return result;
        }
    }

public:
    inline static const std::string name() {
        // The template name + all arguments all mangled.
        // Until I can figure out something better, this will do
        std::stringstream s;
        s << "pwm_" << typeid(wt_pwm_type).name() << '_' << (is_tree ? "tree" : "matrix") << (is_huffman ? "_huffman" : "");
        return s.str();
    }

    inline size_t threads() const {
        return num_threads;
    }

    inline wt_wrapper_pwm(const std::vector<wt_value_type>& v, size_t depth) :
        m_text(v),
        m_wx(wt_pwm_type::compute(v.data(), v.size(), depth)) {}

    inline size_t height() const {
        return m_wx.levels();
    }

    inline size_t access(size_t i) const {
        if constexpr (is_tree) {
            return access_wt(i);
        } else {
            return access_wm(i);
        }
    }

    inline std::string ext(const std::string&) {
        return "-";
    }
};