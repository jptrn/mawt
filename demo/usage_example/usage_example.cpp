#include <huffman/huffman_codes.hpp>
#include <huffman/huffman_codes_operations.hpp>
#include <vector/bit_vector.hpp>
#include <vector/bit_vector_operations.hpp>
#include <wt/parallel/lwt_dd.hpp>
#include <wt/parallel/wt_dd_params.hpp>
#include <wt/lwt.hpp>
#include <wt/lwt_huffman.hpp>
#include <wt/lwt_huffman_operations.hpp>
#include <wt/lwt_operations.hpp>
#include <wt/wm.hpp>
#include <wt/wm_operations.hpp>
#include <wt/wt_params.hpp>

#include <cstdint>
#include <cstddef>
#include <iostream>
#include <vector>
#include <omp.h>

int main(int, char**) {
    std::cout << "Usage example for WTs" << std::endl;


    // Type token declaring which implementation to pick
    using impl_type = wt_params_pext<4, uint16_t>;

    // Other possible type tokens:

    // using impl_type = wt_params_basic;
    // using impl_type = wt_params_no_lut<4, 4, uint16_t>;
    // using impl_type = wt_params_lut<4, 4, 1, uint16_t>;
    // using impl_type = wt_params_shuffle<__m512i>;   // SIMD

    {
        std::cout << "Balanced Levelwise WT:" << std::endl;

        std::vector<uint8_t> values { 0, 1, 32, 4, 0, 4, 7, 117, 244, 13, 255 };
        auto t = lwt::make<uint8_t, impl_type>(values, 8);

        // Access
        for (size_t i = 0; i < lwt_size(t); i++) {
            bit_vector bits = lwt_get(t, i);
            uint8_t value = bit_vector_to<uint8_t>(bits);
            std::cout << (int)value << ' ';
        }
        std::cout << std::endl << std::endl;
    }

    {
        std::cout << "Wavelet Matrix:" << std::endl;

        std::vector<uint8_t> values { 0, 1, 32, 4, 0, 4, 7, 117, 244, 13, 255 };
        auto t = wm::make<uint8_t, impl_type>(values, 8);

        // Access
        for (size_t i = 0; i < wm_size(t); i++) {
            bit_vector bits = wm_get(t, i);
            uint8_t value = bit_vector_to<uint8_t>(bits);
            std::cout << (int)value << ' ';
        }
        std::cout << std::endl << std::endl;
    }

    {
        std::cout << "Huffman-shaped Levelwise Wavelet Tree" << std::endl;

        std::vector<uint8_t> values { 0, 1, 32, 4, 0, 4, 7, 117, 0, 0, 244, 13, 255 };
        huffman_codes_lwt<uint8_t> codes(values);
        auto t = lwt_huffman::make<uint8_t, impl_type>(values, codes);

        // Access
        for (size_t i = 0; i < lwt_huffman_size(t); i++) {
            uint8_t decoded = huffman_codes_decode<uint8_t>(codes, lwt_huffman_get(t, i));
            std::cout << (int)decoded << ' ';
        }
        std::cout << std::endl << std::endl;
    }

    {
        std::cout << "Parallel Wavelet Tree (Domain Decomposition)" << std::endl;

        // Set thread count externally
        constexpr size_t thread_count = 4;
        omp_set_num_threads(thread_count);
        using dd_params_type = wt_dd_params<uint64_t, thread_count>;

        std::vector<uint8_t> values { 0, 1, 32, 4, 0, 4, 7, 117, 0, 0, 244, 13, 255 };
        auto t = lwt_make_dd<uint8_t, impl_type, dd_params_type>(values, 8);

        // Access
        for (size_t i = 0; i < lwt_size(t); i++) {
            bit_vector bits = lwt_get(t, i);
            uint8_t value = bit_vector_to<uint8_t>(bits);
            std::cout << (int)value << ' ';
        }
        std::cout << std::endl << std::endl;
    }
    
    

    std::cout << "Done" << std::endl;
}   
