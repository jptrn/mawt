#pragma once

#include <cstddef>
#include <string>
#include <sstream>

/*
 * Like with wt_params, this is a type token that bundles together several parameters
 * for the domain decomposition construction 
 * 
 * The main parameter is the number of bits to copy during the merging phase, `t_bit_copy`.
 * The thread count is purely for display reasons and has no practical effect - it must be set elsewhere
 */
template <typename t_bit_copy, size_t v_num_threads>
struct wt_dd_params {
    using bit_copy_type = t_bit_copy;
    static constexpr size_t num_threads = v_num_threads;

    static std::string to_string() {
        std::stringstream s;
        s << "dd_merge_" << (sizeof(bit_copy_type) * 8) << "_threads_" << v_num_threads;
        return s.str();
    }
};
