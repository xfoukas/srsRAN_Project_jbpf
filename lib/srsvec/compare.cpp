/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "srsran/srsvec/compare.h"
#include "srsran/srsvec/simd.h"
#include "srsran/support/math/math_utils.h"
#include <numeric>

using namespace srsran;
using namespace srsvec;

const char* srsran::srsvec::detail::find(span<const char> input, const char* value)
{
  // Input index.
  unsigned index = 0;
  char     v     = *value;

#ifdef __AVX2__
  // Advances the input index to either the first SIMD word that contains value or the last index rounded to 32.
  for (unsigned simd_index_end = 32 * (input.size() / 32); index != simd_index_end; index += 32) {
    // Load 32 consecutive words starting at index.
    __m256i simd_input = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(input.data() + index));
    // Compare the 32 words with the value.
    __m256i simd_eq_filler_bit = _mm256_cmpeq_epi8(_mm256_set1_epi8(v), simd_input);
    // Get the MSB of each word.
    unsigned mask = _mm256_movemask_epi8(simd_eq_filler_bit);
    // If it is not zero, it means at least one of the words in the SIMD register is equal to value.
    if (mask > 0) {
      break;
    }
  }
#endif // __AVX2__

#ifdef __ARM_NEON
  bool found = false;
  // Advances the input index to either the first SIMD word that contains value or the last index rounded to 16.
  for (unsigned simd_index_end = 16 * (input.size() / 16); index != simd_index_end; index += 16) {
    // Load 16 consecutive words starting at index.
    int8x16_t simd_input = vld1q_s8(reinterpret_cast<const int8_t*>(input.data() + index));
    // Compare the 16 words with the value.
    uint8x16_t mask_u8 = vceqq_s8(vdupq_n_s8(int8_t(v)), simd_input);
    uint8_t    mask    = vmaxvq_u8(mask_u8);
    if (mask != 0) {
      found = true;
      break;
    }
  }
  // Advances the input index to either the first SIMD word that contains value or the last index rounded to 8.
  for (unsigned simd_index_end = 8 * (input.size() / 8); !found && index != simd_index_end; index += 8) {
    // Load 8 consecutive words starting at index.
    int8x8_t simd_input = vld1_s8(reinterpret_cast<const int8_t*>(input.data() + index));
    // Compare the 8 words with the value.
    uint8x8_t mask_u8 = vceq_s8(vdup_n_s8(int8_t(v)), simd_input);
    uint8_t   mask    = vmaxv_u8(mask_u8);
    if (mask != 0) {
      break;
    }
  }
#endif // __ARM_NEON

  // Keeps iterating from the current index to the end.
  for (; index != input.size(); ++index) {
    // Early return if a word is equal to value.
    if (input[index] == v) {
      return &input[index];
    }
  }

  // There is no word equal to value if the execution reached here.
  return input.end();
}

std::pair<unsigned, float> srsran::srsvec::max_abs_element(span<const cf_t> x)
{
  unsigned i         = 0;
  unsigned len       = x.size();
  unsigned max_index = 0;
  float    max_abs2  = 0;

#if SRSRAN_SIMD_CF_SIZE
  // Prepare range of indexes in SIMD register.
  alignas(SIMD_BYTE_ALIGN) std::array<int32_t, SRSRAN_SIMD_CF_SIZE> simd_vector_max_indexes;
  std::iota(simd_vector_max_indexes.begin(), simd_vector_max_indexes.end(), 0);
  simd_i_t simd_indexes = srsran_simd_i_load(simd_vector_max_indexes.data());

  simd_i_t simd_inc         = srsran_simd_i_set1(SRSRAN_SIMD_CF_SIZE);
  simd_i_t simd_max_indexes = srsran_simd_i_set1(0);

  simd_f_t simd_max_values = srsran_simd_f_set1(-INFINITY);

  for (unsigned simd_end = SRSRAN_SIMD_CF_SIZE * (len / SRSRAN_SIMD_CF_SIZE); i != simd_end; i += SRSRAN_SIMD_CF_SIZE) {
    // Load 2 SIMD words of floats.
    simd_f_t simd_x1 = srsran_simd_f_loadu(reinterpret_cast<const float*>(x.data() + i));
    simd_f_t simd_x2 = srsran_simd_f_loadu(reinterpret_cast<const float*>(x.data() + i + SRSRAN_SIMD_CF_SIZE / 2));

    // Calculates the squares.
    simd_f_t simd_mul1 = srsran_simd_f_mul(simd_x1, simd_x1);
    simd_f_t simd_mul2 = srsran_simd_f_mul(simd_x2, simd_x2);

    // Horizontally add the values in pair, it results in adding the squared real and imaginary parts.
    simd_f_t simd_abs2 = srsran_simd_f_hadd(simd_mul1, simd_mul2);

    // Get SIMD selector mask.
    simd_sel_t res = srsran_simd_f_max(simd_abs2, simd_max_values);

    // Select the indexes and values for the maximum.
    simd_max_indexes = srsran_simd_i_select(simd_max_indexes, simd_indexes, res);
    simd_max_values  = srsran_simd_f_select(simd_max_values, simd_abs2, res);

    // Increment indexes.
    simd_indexes = srsran_simd_i_add(simd_indexes, simd_inc);
  }

  // Store the indexes and values from SIMD registers into arrays.
  alignas(SIMD_BYTE_ALIGN) std::array<float, SRSRAN_SIMD_I_SIZE> simd_vector_max_values;
  srsran_simd_i_store(simd_vector_max_indexes.data(), simd_max_indexes);
  srsran_simd_f_store(simd_vector_max_values.data(), simd_max_values);

  // Find maximum value within the vectors.
  float*   it             = std::max_element(simd_vector_max_values.begin(), simd_vector_max_values.end());
  unsigned simd_max_index = static_cast<unsigned>(it - simd_vector_max_values.begin());
  max_index               = simd_vector_max_indexes[simd_max_index];
  max_abs2                = simd_vector_max_values[simd_max_index];
#endif // SRSRAN_SIMD_CF_SIZE

  for (; i != len; ++i) {
    float abs2 = abs_sq(x[i]);
    if (abs2 > max_abs2) {
      max_index = i;
      max_abs2  = abs2;
    }
  }

  return {max_index, max_abs2};
}

std::pair<unsigned, float> srsran::srsvec::max_element(span<const float> x)
{
  unsigned i         = 0;
  unsigned len       = x.size();
  unsigned max_index = 0;
  float    max_x     = 0;

#if SRSRAN_SIMD_CF_SIZE
  // Prepare range of indexes in SIMD register.
  alignas(SIMD_BYTE_ALIGN) std::array<int32_t, SRSRAN_SIMD_CF_SIZE> simd_vector_max_indexes = {};
  std::iota(simd_vector_max_indexes.begin(), simd_vector_max_indexes.end(), 0);
  simd_i_t simd_indexes = srsran_simd_i_load(simd_vector_max_indexes.data());

  simd_i_t simd_inc         = srsran_simd_i_set1(SRSRAN_SIMD_CF_SIZE);
  simd_i_t simd_max_indexes = srsran_simd_i_set1(0);

  simd_f_t simd_max_values = srsran_simd_f_set1(-INFINITY);

  for (unsigned simd_end = SRSRAN_SIMD_CF_SIZE * (len / SRSRAN_SIMD_CF_SIZE); i != simd_end; i += SRSRAN_SIMD_CF_SIZE) {
    // Load SIMD word of floats.
    simd_f_t simd_x = srsran_simd_f_loadu(reinterpret_cast<const float*>(x.data() + i));

    // Get SIMD selector mask.
    simd_sel_t res = srsran_simd_f_max(simd_x, simd_max_values);

    // Select the indexes and values for the maximum.
    simd_max_indexes = srsran_simd_i_select(simd_max_indexes, simd_indexes, res);
    simd_max_values  = srsran_simd_f_select(simd_max_values, simd_x, res);

    // Increment indexes.
    simd_indexes = srsran_simd_i_add(simd_indexes, simd_inc);
  }

  // Store the indexes and values from SIMD registers into arrays.
  alignas(SIMD_BYTE_ALIGN) std::array<float, SRSRAN_SIMD_I_SIZE> simd_vector_max_values = {};
  srsran_simd_i_store(simd_vector_max_indexes.data(), simd_max_indexes);
  srsran_simd_f_store(simd_vector_max_values.data(), simd_max_values);

  // Find maximum value within the vectors.
  float*   it             = std::max_element(simd_vector_max_values.begin(), simd_vector_max_values.end());
  unsigned simd_max_index = static_cast<unsigned>(it - simd_vector_max_values.begin());
  max_index               = simd_vector_max_indexes[simd_max_index];
  max_x                   = simd_vector_max_values[simd_max_index];
#endif // SRSRAN_SIMD_CF_SIZE

  for (; i != len; ++i) {
    if (x[i] > max_x) {
      max_index = i;
      max_x     = x[i];
    }
  }

  return {max_index, max_x};
}

unsigned srsran::srsvec::count_if_part_abs_greater_than(span<const srsran::cf_t> x, float threshold)
{
  unsigned count = 0;

#ifdef __AVX2__
  __m256i avx_count     = _mm256_setzero_si256();
  __m256  avx_threshold = _mm256_set1_ps(threshold);

  for (unsigned i = 0, simd_end = 8 * (x.size() / 8); i != simd_end; i += 8) {
    // Load input.
    __m256 in0 = _mm256_loadu_ps(reinterpret_cast<const float*>(x.data()));
    __m256 in1 = _mm256_loadu_ps(reinterpret_cast<const float*>(x.data() + 4));

    // Reorder values.
    in0 = _mm256_permute_ps(in0, 0b11011000);
    in1 = _mm256_permute_ps(in1, 0b11011000);

    // Split real and imaginary.
    __m256 re = _mm256_unpacklo_ps(in0, in1);
    __m256 im = _mm256_unpackhi_ps(in0, in1);

    // Absolute.
    re = _mm256_and_ps(re, _mm256_castsi256_ps(_mm256_set1_epi32(2147483647)));
    im = _mm256_and_ps(im, _mm256_castsi256_ps(_mm256_set1_epi32(2147483647)));

    // Check if any part exceeds the threshold.
    __m256i mask_re = _mm256_castps_si256(_mm256_cmp_ps(re, avx_threshold, _CMP_GT_OS));
    __m256i mask_im = _mm256_castps_si256(_mm256_cmp_ps(im, avx_threshold, _CMP_GT_OS));
    __m256i mask    = _mm256_or_si256(mask_re, mask_im);

    // Increment count.
    __m256i avx_count_inc = _mm256_add_epi32(_mm256_set1_epi32(1), avx_count);
    avx_count             = _mm256_blendv_epi8(avx_count, avx_count_inc, mask);

    // Advance input.
    x = x.last(x.size() - 8);
  }

  // Store the resultant count in a vector.
  std::array<int, 8> count_vector;
  _mm256_storeu_si256(reinterpret_cast<__m256i*>(count_vector.data()), avx_count);

  // Sum vales in the vector.
  count = std::accumulate(count_vector.begin(), count_vector.end(), count);
#endif // __AVX2__

  return count + std::count_if(x.begin(), x.end(), [threshold](cf_t sample) {
           return (std::abs(sample.real()) > threshold) || (std::abs(sample.imag()) > threshold);
         });
}
