#pragma once

#include <cstddef>

#include <util/bits.hpp>
#include <vector/bit_vector.hpp>
#include <vector/packed/packed_list_writer.hpp>
#include <wt/wt_twophase_lut.hpp>
#include <wt/wt_params.hpp>
#include <wt/packed/packed_data.hpp>
#include <wt/packed/packed_pext_provider.hpp>
#include <wt/packed/packed_shuffle_provider.hpp>

// Contains static methods for implementing the list splitting and bit extracting methods
// described by Kaneta 2018. The `wt_impl_type` parameter selects the used implementation
// Also provides methods for counting zeros in a packed list to calculate the borders for the next level

template <typename wt_impl_type>
class packed_list_processor {
private:
    constexpr static size_t tau = wt_impl_type::tau;
    constexpr static size_t N = wt_impl_type::N;
    using packed_value_type = wt_impl_type::packed_value_type;

    template <typename value_provider_type>
    static packed_data<packed_value_type> get_values_from_provider(packed_value_type v, size_t alpha, size_t count) {
        const auto& provider = value_provider_type::get();
        auto bits = provider.get_bits(v, alpha);
        auto left_count = provider.get_packed_left_count(v, alpha, count);
        auto right_count = provider.get_packed_right_count(v, alpha, count);
        auto left = provider.get_packed_left(v, alpha);
        auto right = provider.get_packed_right(v, alpha);
        return packed_data<packed_value_type>(left_count, right_count, left, right, bits);
    }

public:
    static void write_bits(size_t i, packed_value_type v, size_t alpha, size_t count, bit_vector& b) {
        if constexpr (wt_impl_type::algorithm == wt_algorithm::NO_LUT) {
            // Process every packed value independently
            auto value_mask = 1ULL << (tau - alpha - 1);
            for (size_t j = 0; j < count; j++) {
                b[i + j] = v & (value_mask << (j * tau));
            }
        } else if constexpr (wt_impl_type::algorithm == wt_algorithm::LUT) {
            constexpr size_t divisions = wt_impl_type::divisions;
            const auto& lut = wt_twophase_lut<tau, N, divisions>::get();
            b.set<N>(i, lut.get_bits(v, alpha));
        } else if constexpr (wt_impl_type::algorithm == wt_algorithm::SPLIT_PEXT) {
            b.set<N>(i, extract_pext<packed_value_type, tau>(v, alpha, count));
        } else if constexpr (wt_impl_type::algorithm == wt_algorithm::SPLIT_SHUFFLE) {
            assert(tau == 8);
            b.set<N>(i, extract_8(v, alpha, count));
        }
    }

    static size_t count_zeros(packed_value_type v, size_t alpha, size_t count) {
        if constexpr (wt_impl_type::algorithm == wt_algorithm::NO_LUT) {
            size_t value = 0;
            // Process every packed value independently
            auto value_mask = 1ULL << (tau - alpha - 1);    
            for (size_t j = 0; j < count; j++) {                
                auto bit = v & (value_mask << (j * tau));
                value += !bit;
            }
            return value;
        } else if constexpr (wt_impl_type::algorithm == wt_algorithm::LUT) {
            constexpr auto divisions = wt_impl_type::divisions;
            const auto& lut = wt_twophase_lut<tau, N, divisions>::get();
            return lut.get_packed_left_count(v, alpha, count);
        } else if constexpr (wt_impl_type::algorithm == wt_algorithm::SPLIT_PEXT) {
            return count_zeros_tau<packed_value_type, tau>(v, alpha, count);
        } else if constexpr (wt_impl_type::algorithm == wt_algorithm::SPLIT_SHUFFLE) {
            assert(tau == 8);
            return count_zeros_8(v, alpha, count);
        }
        return 0;
    }

    static void process_levelwise(
        size_t i,
        packed_value_type v, 
        size_t alpha, 
        size_t count, 
        packed_list_writer<tau, packed_value_type>& write_l, 
        packed_list_writer<tau, packed_value_type>& write_r, 
        bit_vector& b) {
        if constexpr (wt_impl_type::algorithm == wt_algorithm::NO_LUT) {
            // Process every packed value independently
            auto value_mask = 1ULL << (tau - alpha - 1);
            for (size_t j = 0; j < count; j++) {
                // 1.Generate bit vector
                b[i + j] = v & (value_mask << (j * tau));

                // 2.Find out which sublist to put the packed word in, based on the alpha-th bit
                auto packed_word = v >> (tau * j);
                packed_word &= mask(tau);
                if (!(packed_word & value_mask)) {
                    write_l.write(1, packed_word);
                } else {
                    write_r.write(1, packed_word);
                }
            }
        } else {
            packed_data<packed_value_type> values;
            if constexpr (wt_impl_type::algorithm == wt_algorithm::LUT) {
                constexpr size_t divisions = wt_impl_type::divisions;
                values = get_values_from_provider<wt_twophase_lut<tau, N, divisions>>(v, alpha, count);
            } else if constexpr (wt_impl_type::algorithm == wt_algorithm::SPLIT_PEXT) {
                values = get_packed_data_pext_pext<packed_value_type, tau>(v, alpha, count);
            } else if constexpr (wt_impl_type::algorithm == wt_algorithm::SPLIT_SHUFFLE) {
                values = get_packed_data_shuffle(v, alpha, count);
            }

            b.set<N>(i, values.bits());
            write_l.write(values.left_count(), values.left());
            write_r.write(values.right_count(), values.right());
        }
    }
};