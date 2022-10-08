#pragma once

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <benchmark_wt/benchmark_params.hpp>
#include <benchmark_wt/benchmark_results.hpp>
#include <benchmark_util.hpp>

#include <wt/parallel/wt_dd_params.hpp>
#include <wt/wt_params.hpp>

#include <util/timer.hpp>
#include <spacer.hpp>

#include <format/format_string.hpp>

#include <wrappers/wt_wrapper_dummy.hpp>
#include <wrappers/lwt_wrapper.hpp>
#include <wrappers/lwt_wrapper_dd.hpp>
#include <wrappers/lwt_wrapper_huffman.hpp>
#include <wrappers/wm_wrapper.hpp>
#include <wrappers/wm_wrapper_dd.hpp>
#include <wrappers/wm_wrapper_huffman.hpp>
#include <wrappers/wt_wrapper_pwm.hpp>

#include <omp.h>

// This class is used to allow benchmarks to run with some arbitrary type of user data

template <typename value_type>
class benchmark_execution {
private:
    const benchmark_params& m_params;
    benchmark_results& m_results;

    constexpr static bool TEST_REGULAR = true;
    constexpr static bool TEST_HUFFMAN = true;
    constexpr static bool TEST_DD = true;


    // Parameters about current text
    const std::vector<value_type>& m_text;
    size_t m_alpha_size;
    size_t m_alpha_bits;
    size_t m_iteration;
    size_t m_ds_order = 0;
    std::string m_filename; // "-" if randomly generated

    // Verification variables

    template <typename wt_type>
    inline void verify(const wt_type& wt) {
        std::vector<size_t> values {
            wt.access(0),
            wt.access(m_text.size() / 2),
            wt.access(m_text.size() - 1)
        };
        std::cout << "Sentinel values: " 
            << values[0] << ", " 
            << values[1] << ", " 
            << values[2] << std::endl;

        // Verify current results against known results
        for (size_t i = 0; i < values.size(); i++) {
            if (m_results.results[i] != values[i]) {
                m_results.failed_implementations.emplace(wt_type::name());
                if (!m_results.failed) {
                    std::cout << FMT.format("!! RESULT MISMATCH !!", fmt_color::RED, fmt_color::NONE, fmt_fx::BOLD) << std::endl;
                    std::stringstream s;
                    s << "Iteration: " << (m_iteration + 1) << " of " << m_params.iterations << ", Implementation: " << wt_type::name() << std::endl
                        << "Expected: " << m_results.results[i] << ", got: " << values[i];
                    m_results.failure_msg = s.str();
                    std::cout << m_results.failure_msg << std::endl;
                    if (m_params.verify) {
                        // Fail fast    
                        std::exit(EXIT_FAILURE);
                    } else {
                        // Continue, but report failure message at end
                        // Also, only count the first mismatch
                        m_results.failed = true;
                    }
                }
            }
        } 
    }

    // -- The main Benchmark function --
    template <typename wt_type>
    void bench() {
        std::cout << "Benchmark: " << wt_type::name() << std::endl;

        timer t;
        spacer s;

        s.reset();
        t.reset();

        // TODO: Some implementations might destroy their input - I should create a copy of the text
        // iff that is the case - but only if I actually have an algorithm that requires it though
        wt_type wt(m_text, m_alpha_bits);

        auto time_used = t.elapsed();
        auto space_peak = s.space_peak();
        auto space_used = s.space_used();

        std::cout << FMT.format("RESULT", fmt_color::BLUE, fmt_color::NONE, fmt_fx::BOLD)
                << " iteration=" << m_iteration
                << " type=" << wt_type::name()
                << " operation=ctor"
                << " file=" << m_filename
                << " size=" << m_text.size()
                << " height=" << wt.height()
                << " alpha_bits=" << m_alpha_bits
                << " alphabet_size=" << m_alpha_size
                << " time_in_s=" << time_used
                << " threads=" << wt.threads()
                << " space_in_bytes=" << space_used
                << " working_space=" << space_peak
                << " mode=" << (m_params.random_seed ? "random" : "fixed")
                << " seed=" << m_params.seed
                << " ds_order=" << m_ds_order++;
        for (auto ext : m_params.extra_keys) {
            std::cout << " ext_" << ext << "=" << wt.ext(ext);
        }

        std::cout << std::endl;
        verify(wt);
    }

    template<typename merge_value_type, size_t v_num_threads>
    void run_domain_decomposition() {
        omp_set_num_threads(v_num_threads);

        // LWT
        // b = 16
        bench<lwt_wrapper_dd<value_type, wt_params_lut<4, 4, 1, uint16_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<lwt_wrapper_dd<value_type, wt_params_pext<2, uint16_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<lwt_wrapper_dd<value_type, wt_params_pext<4, uint16_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<lwt_wrapper_dd<value_type, wt_params_pext<8, uint16_t>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // b = 32
        bench<lwt_wrapper_dd<value_type, wt_params_lut<4, 8, 2, uint32_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<lwt_wrapper_dd<value_type, wt_params_pext<2, uint32_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<lwt_wrapper_dd<value_type, wt_params_pext<4, uint32_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<lwt_wrapper_dd<value_type, wt_params_pext<8, uint32_t>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // b = 64
        bench<lwt_wrapper_dd<value_type, wt_params_lut<4, 16, 4, uint64_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<lwt_wrapper_dd<value_type, wt_params_pext<2, uint64_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<lwt_wrapper_dd<value_type, wt_params_pext<4, uint64_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<lwt_wrapper_dd<value_type, wt_params_pext<8, uint64_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<lwt_wrapper_dd<value_type, wt_params_shuffle<uint64_t>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // b = 128
        bench<lwt_wrapper_dd<value_type, wt_params_shuffle<__m128i>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // b = 256
        bench<lwt_wrapper_dd<value_type, wt_params_shuffle<__m256i>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // b = 512
        bench<lwt_wrapper_dd<value_type, wt_params_shuffle<__m512i>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // PWM
        bench<wt_wrapper_pwm<value_type, wx_dd_pc<value_type, true>, v_num_threads>>();
        bench<wt_wrapper_pwm<value_type, wx_dd_pc_ss<value_type, true>, v_num_threads>>();
        bench<wt_wrapper_pwm<value_type, wx_dd_ps<value_type, true>, v_num_threads>>();

        // WM
        // b = 16
        bench<wm_wrapper_dd<value_type, wt_params_lut<4, 4, 1, uint16_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<wm_wrapper_dd<value_type, wt_params_pext<2, uint16_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<wm_wrapper_dd<value_type, wt_params_pext<4, uint16_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<wm_wrapper_dd<value_type, wt_params_pext<8, uint16_t>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // b = 32
        bench<wm_wrapper_dd<value_type, wt_params_lut<4, 8, 2, uint32_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<wm_wrapper_dd<value_type, wt_params_pext<2, uint32_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<wm_wrapper_dd<value_type, wt_params_pext<4, uint32_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<wm_wrapper_dd<value_type, wt_params_pext<8, uint32_t>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // b = 64
        bench<wm_wrapper_dd<value_type, wt_params_lut<4, 16, 4, uint64_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<wm_wrapper_dd<value_type, wt_params_pext<2, uint64_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<wm_wrapper_dd<value_type, wt_params_pext<4, uint64_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<wm_wrapper_dd<value_type, wt_params_pext<8, uint64_t>, wt_dd_params<merge_value_type, v_num_threads>>>();
        bench<wm_wrapper_dd<value_type, wt_params_shuffle<uint64_t>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // b = 128
        bench<wm_wrapper_dd<value_type, wt_params_shuffle<__m128i>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // b = 256
        bench<wm_wrapper_dd<value_type, wt_params_shuffle<__m256i>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // b = 512
        bench<wm_wrapper_dd<value_type, wt_params_shuffle<__m512i>, wt_dd_params<merge_value_type, v_num_threads>>>();

        // PWM
        bench<wt_wrapper_pwm<value_type, wx_dd_pc<value_type, false>, v_num_threads>>();
        bench<wt_wrapper_pwm<value_type, wx_dd_pc_ss<value_type, false>, v_num_threads>>();
        bench<wt_wrapper_pwm<value_type, wx_dd_ps<value_type, false>, v_num_threads>>();
    }

public:
    benchmark_execution(
        const benchmark_params& params, 
        benchmark_results& results,
        const std::vector<value_type>& text, 
        size_t alpha_size, 
        size_t alpha_bits, 
        size_t m_iteration,
        const std::string& filepath = "-") : 

        m_params(params),
        m_results(results),
        m_text(text),
        m_alpha_size(alpha_size),
        m_alpha_bits(alpha_bits),
        m_iteration(m_iteration),
        m_filename(filepath) {}

    void run() {
        if (m_results.results.empty()) {
            m_results.results.push_back(m_text[0]);
            m_results.results.push_back(m_text[m_text.size() / 2]);
            m_results.results.push_back(m_text[m_text.size() - 1]);
            std::cout << FMT.format("[ EXPECTED ]", fmt_color::WHITE, fmt_color::NONE, fmt_fx::BOLD) << ": " 
                << m_results.results[0] << ", " 
                << m_results.results[1] << ", " 
                << m_results.results[2] << std::endl;
        } 

        omp_set_num_threads(1);

        if constexpr (TEST_REGULAR) {
            // LWT
            // b = 16
            bench<lwt_wrapper<value_type, wt_params_lut<4, 4, 1, uint16_t>>>();
            bench<lwt_wrapper<value_type, wt_params_pext<2, uint16_t>>>();
            bench<lwt_wrapper<value_type, wt_params_pext<4, uint16_t>>>();
            bench<lwt_wrapper<value_type, wt_params_pext<8, uint16_t>>>();

            // b = 32
            bench<lwt_wrapper<value_type, wt_params_lut<4, 8, 2, uint32_t>>>();
            bench<lwt_wrapper<value_type, wt_params_pext<2, uint32_t>>>();
            bench<lwt_wrapper<value_type, wt_params_pext<4, uint32_t>>>();
            bench<lwt_wrapper<value_type, wt_params_pext<8, uint32_t>>>();

            // b = 64
            bench<lwt_wrapper<value_type, wt_params_lut<4, 16, 4, uint64_t>>>();
            bench<lwt_wrapper<value_type, wt_params_pext<2, uint64_t>>>();
            bench<lwt_wrapper<value_type, wt_params_pext<4, uint64_t>>>();
            bench<lwt_wrapper<value_type, wt_params_pext<8, uint64_t>>>();
            bench<lwt_wrapper<value_type, wt_params_shuffle<uint64_t>>>();

            // b = 128
            bench<lwt_wrapper<value_type, wt_params_shuffle<__m128i>>>();

            // b = 256
            bench<lwt_wrapper<value_type, wt_params_shuffle<__m256i>>>();

            // b = 512
            bench<lwt_wrapper<value_type, wt_params_shuffle<__m512i>>>();

            // PWM
            bench<wt_wrapper_pwm<value_type, wx_pc<value_type, true>>>();
            bench<wt_wrapper_pwm<value_type, wx_pc_ss<value_type, true>>>();
            bench<wt_wrapper_pwm<value_type, wx_ps<value_type, true>>>();

            // WM
            // b = 16
            bench<wm_wrapper<value_type, wt_params_lut<4, 4, 1, uint16_t>>>();
            bench<wm_wrapper<value_type, wt_params_pext<2, uint16_t>>>();
            bench<wm_wrapper<value_type, wt_params_pext<4, uint16_t>>>();
            bench<wm_wrapper<value_type, wt_params_pext<8, uint16_t>>>();

            // b = 32
            bench<wm_wrapper<value_type, wt_params_lut<4, 8, 2, uint32_t>>>();
            bench<wm_wrapper<value_type, wt_params_pext<2, uint32_t>>>();
            bench<wm_wrapper<value_type, wt_params_pext<4, uint32_t>>>();
            bench<wm_wrapper<value_type, wt_params_pext<8, uint32_t>>>();

            // b = 64
            bench<wm_wrapper<value_type, wt_params_lut<4, 16, 4, uint64_t>>>();
            bench<wm_wrapper<value_type, wt_params_pext<2, uint64_t>>>();
            bench<wm_wrapper<value_type, wt_params_pext<4, uint64_t>>>();
            bench<wm_wrapper<value_type, wt_params_pext<8, uint64_t>>>();
            bench<wm_wrapper<value_type, wt_params_shuffle<uint64_t>>>();

            // b = 128
            bench<wm_wrapper<value_type, wt_params_shuffle<__m128i>>>();

            // b = 256
            bench<wm_wrapper<value_type, wt_params_shuffle<__m256i>>>();

            // b = 512
            bench<wm_wrapper<value_type, wt_params_shuffle<__m512i>>>();

            // PWM
            bench<wt_wrapper_pwm<value_type, wx_pc<value_type, false>>>();
            bench<wt_wrapper_pwm<value_type, wx_pc_ss<value_type, false>>>();
            bench<wt_wrapper_pwm<value_type, wx_ps<value_type, false>>>();
        }

        if constexpr (TEST_HUFFMAN) {
            // LWT
            // b = 16
            bench<lwt_wrapper_huffman<value_type, wt_params_pext<2, uint16_t>>>();
            bench<lwt_wrapper_huffman<value_type, wt_params_pext<4, uint16_t>>>();
            bench<lwt_wrapper_huffman<value_type, wt_params_pext<8, uint16_t>>>();

            // b = 32
            bench<lwt_wrapper_huffman<value_type, wt_params_pext<2, uint32_t>>>();
            bench<lwt_wrapper_huffman<value_type, wt_params_pext<4, uint32_t>>>();
            bench<lwt_wrapper_huffman<value_type, wt_params_pext<8, uint32_t>>>();

            // b = 64
            bench<lwt_wrapper_huffman<value_type, wt_params_pext<2, uint64_t>>>();
            bench<lwt_wrapper_huffman<value_type, wt_params_pext<4, uint64_t>>>();
            bench<lwt_wrapper_huffman<value_type, wt_params_pext<8, uint64_t>>>();
            bench<lwt_wrapper_huffman<value_type, wt_params_shuffle<uint64_t>>>();

            // b = 128
            bench<lwt_wrapper_huffman<value_type, wt_params_shuffle<__m128i>>>();

            // b = 256
            bench<lwt_wrapper_huffman<value_type, wt_params_shuffle<__m256i>>>();

            // b = 512
            bench<lwt_wrapper_huffman<value_type, wt_params_shuffle<__m512i>>>();

            // PWM
            bench<wt_wrapper_pwm<value_type, wx_huff_pc<value_type, true>>>();
            bench<wt_wrapper_pwm<value_type, wx_huff_pc_ss<value_type, true>>>();
            bench<wt_wrapper_pwm<value_type, wx_huff_ps<value_type, true>>>();

            // WM
            // b = 16
            bench<wm_wrapper_huffman<value_type, wt_params_pext<2, uint16_t>>>();
            bench<wm_wrapper_huffman<value_type, wt_params_pext<4, uint16_t>>>();
            bench<wm_wrapper_huffman<value_type, wt_params_pext<8, uint16_t>>>();

            // b = 32
            bench<wm_wrapper_huffman<value_type, wt_params_pext<2, uint32_t>>>();
            bench<wm_wrapper_huffman<value_type, wt_params_pext<4, uint32_t>>>();
            bench<wm_wrapper_huffman<value_type, wt_params_pext<8, uint32_t>>>();

            // b = 64
            bench<wm_wrapper_huffman<value_type, wt_params_pext<2, uint64_t>>>();
            bench<wm_wrapper_huffman<value_type, wt_params_pext<4, uint64_t>>>();
            bench<wm_wrapper_huffman<value_type, wt_params_pext<8, uint64_t>>>();
            bench<wm_wrapper_huffman<value_type, wt_params_shuffle<uint64_t>>>();

            // b = 128
            bench<wm_wrapper_huffman<value_type, wt_params_shuffle<__m128i>>>();

            // b = 256
            bench<wm_wrapper_huffman<value_type, wt_params_shuffle<__m256i>>>();

            // b = 512
            bench<wm_wrapper_huffman<value_type, wt_params_shuffle<__m512i>>>();

            // PWM
            bench<wt_wrapper_pwm<value_type, wx_huff_pc<value_type, false>>>();
            bench<wt_wrapper_pwm<value_type, wx_huff_pc_ss<value_type, false>>>();
            bench<wt_wrapper_pwm<value_type, wx_huff_ps<value_type, false>>>();
        }

        if constexpr (TEST_DD) {
            run_domain_decomposition<__m512i, 2>();
            run_domain_decomposition<__m512i, 4>();
            run_domain_decomposition<__m512i, 8>();
            run_domain_decomposition<__m512i, 16>();
        }
    }
};
