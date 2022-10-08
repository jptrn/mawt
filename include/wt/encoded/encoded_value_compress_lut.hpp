#pragma once

#include <cstddef>

template <typename enc_packed_value_type, size_t tau> 
class encoded_value_compress_lut {
private:
    enc_packed_value_type m_data[tau];
    static_assert((sizeof(enc_packed_value_type) * 8) % tau == 0);

    inline constexpr encoded_value_compress_lut() : m_data() {
        for (size_t alpha = 0; alpha < tau; alpha++) {
            enc_packed_value_type value = 0;
            for (size_t j = 0; j < (sizeof(enc_packed_value_type) * 8) / tau; j++) {
                value |= (tau - alpha - 1) << (j * tau);
            }
            m_data[alpha] = value;
        }
    }

    constexpr static encoded_value_compress_lut singleton_instance = encoded_value_compress_lut();

public:
    encoded_value_compress_lut(const encoded_value_compress_lut&) = delete;
    encoded_value_compress_lut(const encoded_value_compress_lut&&) = delete;
    encoded_value_compress_lut& operator=(const encoded_value_compress_lut&) = delete;
    encoded_value_compress_lut& operator=(const encoded_value_compress_lut&&) = delete;

    inline constexpr enc_packed_value_type get_add_mask(size_t alpha) const {
        return m_data[alpha];
    }

    inline constexpr static const encoded_value_compress_lut& get() {
        static_assert(singleton_instance.get_add_mask(0) != 0);
        return singleton_instance;
    }
};