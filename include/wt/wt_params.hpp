#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <sstream>

/*
 * Defines a type token that is used to select which Wavelet Tree construction algorithm to choose,
 * along with applicable hyperparameters for the algorithm, such as N and tau.
 *
 * Each token must define at least a static `wt_algorithm` field (see the enum below), and a
 * `to_string()` method. Other members and methods must be provided as they are required by their
 * respective implementations.
 */

// Allow any arbitrary combination of list splitting / bit exctracting method,
// including lookups etc.

enum wt_algorithm {
    BASIC,          // Naive construction
    NO_LUT,         // Two-Phase construction, but process each element of a packed list individually
    LUT,            // Use look-up tables
    SPLIT_PEXT,     // Kaneta 2018: pext for bit packing, pext for list splitting
    SPLIT_SHUFFLE   // Kaneta 2018: pext for bit packing, pshufb for list splitting
};

struct wt_params_basic {
    static constexpr wt_algorithm algorithm = wt_algorithm::BASIC;
    
    static std::string to_string() {
        return "basic";
    }
};

template <size_t v_tau, size_t v_n, typename t_packed_value = uint64_t>
struct wt_params_no_lut {
    static constexpr wt_algorithm algorithm = wt_algorithm::NO_LUT;

    static constexpr size_t N = v_n;
    static constexpr size_t tau = v_tau;

    static std::string to_string() {
        std::stringstream s;
        s << "no_lut_" << N << "_" << tau;
        return s.str();
    }

    using packed_value_type = t_packed_value;

    static_assert(sizeof(packed_value_type) * 8 == tau * N);
};

template <size_t v_tau, size_t v_n, size_t v_divisions = 1, typename t_packed_value = uint64_t>
struct wt_params_lut {
    static constexpr wt_algorithm algorithm = wt_algorithm::LUT;

    static constexpr size_t N = v_n;
    static constexpr size_t tau = v_tau;
    static constexpr size_t divisions = v_divisions;

    static std::string to_string() {
        std::stringstream s;
        s << "lut_" << N << "_" << tau << "_" << divisions;
        return s.str();
    }

    using packed_value_type = t_packed_value;

    static_assert(sizeof(packed_value_type) * 8 == tau * N);
};

// b <= 64, uses pext for value splitting
template <size_t v_tau, typename t_packed_value>
struct wt_params_pext {
    static constexpr wt_algorithm algorithm = wt_algorithm::SPLIT_PEXT;

    static constexpr size_t tau = v_tau;
    using packed_value_type = t_packed_value;
    static constexpr size_t N = (sizeof(packed_value_type) * 8) / tau;

    static std::string to_string() {
        std::stringstream s;
        s << "pext_" << N << "_" << tau;
        return s.str();
    }

    static_assert(sizeof(packed_value_type) <= 8);
    static_assert((sizeof(packed_value_type) * 8) % tau == 0);
};

// tau = 8, uses pshufb/vpermb vor value splitting and pext/vpshufbitqmb for extracting, SIMD capable
template <typename t_packed_value = uint64_t>
struct wt_params_shuffle {
    static constexpr wt_algorithm algorithm = wt_algorithm::SPLIT_SHUFFLE;

    static constexpr size_t tau = 8;
    using packed_value_type = t_packed_value;
    static constexpr size_t N = (sizeof(packed_value_type) * 8) / tau;;

    static std::string to_string() {
        std::stringstream s;
        s << "shuffle_" << N << "_" << tau;
        return s.str();
    }
};
