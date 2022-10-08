#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <sstream>
#include <vector>

#include <vector/bit_vector_operations.hpp>
#include <wt/lwt.hpp>
#include <wt/lwt_operations.hpp>


template <typename value_type, typename wt_impl_type>
class lwt_wrapper {
private:
    lwt m_wt;

public:
    lwt_wrapper(const std::vector<value_type>& s, size_t bit_depth) :
        m_wt(lwt::make<value_type, wt_impl_type>(s, bit_depth)) { }
    
    inline static const std::string name() {
        std::stringstream s;
        s << "lwt_" << wt_impl_type::to_string();
        return s.str();
    }

    size_t height() const {
        return lwt_height(m_wt);
    }

    inline size_t threads() const {
        return 1;
    }

    size_t access(size_t i) const {
        return bit_vector_to<size_t>(lwt_get(m_wt, i));
    }

    inline std::string ext(const std::string&) {
        return "-";
    }
};