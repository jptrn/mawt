#pragma once

#include <cstddef>
#include <cstdint>

#include <immintrin.h>

const __m128i bit_n_128_8[] = {
    _mm_set1_epi8(0x01),       // ~ L in Kaneta's paper
    _mm_set1_epi8(0x02),
    _mm_set1_epi8(0x04),
    _mm_set1_epi8(0x08),
    _mm_set1_epi8(0x10),
    _mm_set1_epi8(0x20),
    _mm_set1_epi8(0x40),
    _mm_set1_epi8(0x80)        // ~ H
};

const __m256i bit_n_256_8[] = {
    _mm256_set1_epi8(0x01),
    _mm256_set1_epi8(0x02),
    _mm256_set1_epi8(0x04),
    _mm256_set1_epi8(0x08),
    _mm256_set1_epi8(0x10),
    _mm256_set1_epi8(0x20),
    _mm256_set1_epi8(0x40),
    _mm256_set1_epi8(0x80)  
};

const __m512i bit_n_512_8[] = {
    _mm512_set1_epi8(0x01),
    _mm512_set1_epi8(0x02),
    _mm512_set1_epi8(0x04),
    _mm512_set1_epi8(0x08),
    _mm512_set1_epi8(0x10),
    _mm512_set1_epi8(0x20),
    _mm512_set1_epi8(0x40),
    _mm512_set1_epi8(0x80)  
};

// Mask used for _mm_bitshuffle_epi64_mask
const __m128i bitshuf_mask_128_8[] = {
    _mm_set1_epi64((__m64)0x38'30'28'20'18'10'08'00),
    _mm_set1_epi64((__m64)0x39'31'29'21'19'11'09'01),
    _mm_set1_epi64((__m64)0x3A'32'2A'22'1A'12'0A'02),
    _mm_set1_epi64((__m64)0x3B'33'2B'23'1B'13'0B'03),
    _mm_set1_epi64((__m64)0x3C'34'2C'24'1C'14'0C'04),
    _mm_set1_epi64((__m64)0x3D'35'2D'25'1D'15'0D'05),
    _mm_set1_epi64((__m64)0x3E'36'2E'26'1E'16'0E'06),
    _mm_set1_epi64((__m64)0x3F'37'2F'27'1F'17'0F'07)
};

const __m256i bitshuf_mask_256_8[] = {
    _mm256_set1_epi64x(0x38'30'28'20'18'10'08'00),
    _mm256_set1_epi64x(0x39'31'29'21'19'11'09'01),
    _mm256_set1_epi64x(0x3A'32'2A'22'1A'12'0A'02),
    _mm256_set1_epi64x(0x3B'33'2B'23'1B'13'0B'03),
    _mm256_set1_epi64x(0x3C'34'2C'24'1C'14'0C'04),
    _mm256_set1_epi64x(0x3D'35'2D'25'1D'15'0D'05),
    _mm256_set1_epi64x(0x3E'36'2E'26'1E'16'0E'06),
    _mm256_set1_epi64x(0x3F'37'2F'27'1F'17'0F'07)
};

const __m512i bitshuf_mask_512_8[] = {
    _mm512_set1_epi64(0x38'30'28'20'18'10'08'00),
    _mm512_set1_epi64(0x39'31'29'21'19'11'09'01),
    _mm512_set1_epi64(0x3A'32'2A'22'1A'12'0A'02),
    _mm512_set1_epi64(0x3B'33'2B'23'1B'13'0B'03),
    _mm512_set1_epi64(0x3C'34'2C'24'1C'14'0C'04),
    _mm512_set1_epi64(0x3D'35'2D'25'1D'15'0D'05),
    _mm512_set1_epi64(0x3E'36'2E'26'1E'16'0E'06),
    _mm512_set1_epi64(0x3F'37'2F'27'1F'17'0F'07)
};
