#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <sstream>
#include <vector>

#include <vector/bit_vector_operations.hpp>
#include <wt/wm.hpp>
#include <wt/wm_operations.hpp>

template <typename value_type, typename wt_impl_type>
class wm_wrapper {
private:
    wm m_wm;

public:
    wm_wrapper(const std::vector<value_type>& s, size_t bit_depth) :
        m_wm(wm::make<value_type, wt_impl_type>(s, bit_depth)) { }
    
    inline static const std::string name() {
        std::stringstream s;
        s << "wm_" << wt_impl_type::to_string();
        return s.str();
    }

    inline size_t threads() const {
        return 1;
    }

    size_t height() const {
        return wm_height(m_wm);
    }

    size_t access(size_t i) const {
        return bit_vector_to<size_t>(wm_get(m_wm, i));
    }

    inline std::string ext(const std::string&) {
        return "-";
    }
};