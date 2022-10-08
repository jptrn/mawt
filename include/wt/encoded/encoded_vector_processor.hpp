#pragma once

#include <cstddef>
#include <tuple>

#include <util/bits.hpp>
#include <vector/bit_vector.hpp>
#include <vector/encoded/encoded_vector.hpp>
#include <vector/encoded/encoded_vector_writer.hpp>
#include <wt/wt_params.hpp>
#include <wt/encoded/encoded_value_compressor_bit_extract.hpp>
#include <wt/encoded/encoded_value_compressor_shuffle.hpp>
#include <wt/encoded/encoded_pext_provider.hpp>
#include <wt/encoded/encoded_shuffle_provider.hpp>
#include <wt/encoded/packed_enc_data.hpp>
#include <wt/packed/packed_pext_provider.hpp>
#include <wt/packed/packed_shuffle_provider.hpp>

// See `encoded_vector_processor.hpp`, this is the equivalent class for the encoded_vector
template <typename wt_impl_type>
class encoded_vector_processor {
private:
    constexpr static size_t tau = wt_impl_type::tau;
    constexpr static size_t N = wt_impl_type::N;
    using packed_enc_value_type = wt_impl_type::packed_value_type;

public:
    static inline bool in_next_packed(size_t code_length, size_t alpha) {
        return code_length > alpha;
    }

    static std::tuple<size_t, size_t> count_bits(packed_enc_value_type v, packed_enc_value_type l, size_t alpha, size_t count) {
        if constexpr (wt_impl_type::algorithm == wt_algorithm::NO_LUT) {
            size_t zero_count = 0;
            size_t one_count = 0;
            // Process every packed value independently
            auto value_mask = 1ULL << (tau - alpha - 1);
            for (size_t j = 0; j < count; j++) {
                if (in_next_packed(l & mask(tau), alpha)) {
                    auto bit = v & (value_mask << (j * tau));
                    (!bit ? zero_count : one_count)++;
                }
                l >>= tau;
            }
            return { zero_count, one_count };
        } else if constexpr (wt_impl_type::algorithm == wt_algorithm::SPLIT_PEXT) {
            // we just compress the values and then treat them as normal - if this is ok performance wise, I'm not gonna change it
            auto [ compressed, lengths, new_count ] = compress_values_bit_extract<packed_enc_value_type, tau>(v, l, alpha, count);
            size_t zeros = count_zeros_tau<packed_enc_value_type, tau>(compressed, alpha, new_count);
            return { zeros, new_count - zeros };
        } else if constexpr (wt_impl_type::algorithm == wt_algorithm::SPLIT_SHUFFLE) {
            auto [ compressed, lengths, new_count ] = compress_values_shuffle<packed_enc_value_type, N>(v, l, alpha, count);
            size_t zeros = count_zeros_8<packed_enc_value_type>(compressed, alpha, new_count);
            return { zeros, new_count - zeros };
        }
        return { 0, 0 };
    }

    static void process_levelwise(
        size_t i,
        packed_enc_value_type v,
        packed_enc_value_type l,
        size_t alpha, 
        size_t count, 
        encoded_vector_writer<packed_enc_value_type, tau>& write_l, 
        encoded_vector_writer<packed_enc_value_type, tau>& write_r, 
        bit_vector& b) {

        if constexpr (wt_impl_type::algorithm == wt_algorithm::NO_LUT) {
            // Process every packed value independently
            auto value_mask = 1ULL << (tau - alpha - 1);
            for (size_t j = 0; j < count; j++) {
                b[i + j] = v & (value_mask << (j * tau));
                auto packed_word = (v >> (tau * j)) & mask(tau);
                auto packed_length = (l >> (tau * j)) & mask(tau);
                if (in_next_packed(packed_length, alpha)) {
                    if (!(packed_word & value_mask)) {
                        write_l.write(1, packed_word, packed_length);
                    } else {
                        write_r.write(1, packed_word, packed_length);
                    }
                }
            }
        } else {
            packed_enc_data<packed_enc_value_type> values; 
            size_t bits = 0;           
            if constexpr (wt_impl_type::algorithm == wt_algorithm::SPLIT_PEXT) {
                bits = extract_pext<packed_enc_value_type, tau>(v, alpha, count);
                auto [ compressed, lengths, new_count ] = compress_values_bit_extract<packed_enc_value_type, tau>(v, l, alpha, count);
                values = split_enc_pext<packed_enc_value_type, tau>(compressed, lengths, alpha, new_count);
            } else if constexpr (wt_impl_type::algorithm == wt_algorithm::SPLIT_SHUFFLE) {
                bits = extract_8<packed_enc_value_type>(v, alpha, count);
                auto [ compressed, lengths, new_count ] = compress_values_shuffle<packed_enc_value_type, N>(v, l, alpha, count);
                values = split_enc_shuffle<packed_enc_value_type>(compressed, lengths, alpha, new_count);
            }

            b.set<N>(i, bits);
            write_l.write(values.left_count(), values.left(), values.left_lengths());
            write_r.write(values.right_count(), values.right(), values.right_lengths());
        }
    }
};