#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <sstream>
#include <vector>

#include <huffman/huffman_codes.hpp>
#include <huffman/huffman_codes_operations.hpp>
#include <util/time_measure.hpp>
#include <wt/wm_huffman.hpp>
#include <wt/wm_huffman_operations.hpp>


template <typename value_type, typename wt_impl_type>
class wm_wrapper_huffman {
private:
    size_t m_alpha_bits = 0;

    time_measure m_time;
    huffman_codes_wm<value_type> m_codes;
    wm_huffman m_wt;

    inline huffman_codes_wm<value_type> get_codes(const std::vector<value_type>& text) {
        m_time.start("get_huffman_codes");
        auto huffman_codes = huffman_codes_wm<value_type>(text);
        m_time.end("get_huffman_codes");
        return huffman_codes;
    }

    inline wm_huffman get_wm(const std::vector<value_type>& text) {
        m_time.start("make_tree");
        auto wm = wm_huffman::make<value_type, wt_impl_type>(text, m_codes);
        m_time.end("make_tree");
        return wm;
    }

public:
    wm_wrapper_huffman(const std::vector<value_type>& s, size_t) :
        m_time(),
        m_codes(get_codes(s)),
        m_wt(get_wm(s)) { }
    
    inline static const std::string name() {
        std::stringstream s;
        s << "wm_huffman_" << wt_impl_type::to_string();
        return s.str();
    }

    size_t height() const {
        return wm_huffman_height(m_wt);
    }

    inline size_t threads() const {
        return 1;
    }

    size_t access(size_t i) const {
        return huffman_codes_decode<value_type>(m_codes, wm_huffman_get(m_wt, i));
    }

    inline std::string ext(const std::string& key) {
        std::stringstream s;     
        s << m_time.get(key);
        return s.str();
    }
};