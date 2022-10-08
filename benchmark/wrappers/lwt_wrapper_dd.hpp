#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <sstream>
#include <vector>

#include <util/time_measure.hpp>
#include <vector/bit_vector_operations.hpp>
#include <wt/parallel/lwt_dd.hpp>
#include <wt/lwt.hpp>
#include <wt/lwt_operations.hpp>


template <typename value_type, typename wt_impl_type, typename wt_dd_impl_type>
class lwt_wrapper_dd {
private:
    time_measure m_time;
    lwt m_wt;

public:
    lwt_wrapper_dd(const std::vector<value_type>& s, size_t bit_depth) :
        m_time(),
        m_wt(lwt_make_dd<value_type, wt_impl_type, wt_dd_impl_type>(s, bit_depth, m_time)) { }
    
    inline static const std::string name() {
        std::stringstream s;
        s << "lwt_dd_" << wt_impl_type::to_string() << "_" << wt_dd_impl_type::to_string();
        return s.str();
    }

    size_t height() const {
        return lwt_height(m_wt);
    }

    inline size_t threads() const {
        return wt_dd_impl_type::num_threads;
    }

    size_t access(size_t i) const {
        return bit_vector_to<size_t>(lwt_get(m_wt, i));
    }

    double elapsed(const std::string& key) {
        return m_time.get(key);
    }

    inline std::string ext(const std::string& key) {
        std::stringstream s;
        s << elapsed(key);
        return s.str();
    }
};