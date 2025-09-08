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

#pragma once

#include <immintrin.h>

#ifndef __AVX512F__
#error "Architecture missmatch. Missing avx512f."
#endif // __AVX512F__

#ifndef __AVX512BW__
#error "Architecture missmatch. Missing avx512bw."
#endif // __AVX512BW__

#if defined(__GNUC__) && (__GNUC__ <= 9)
#error "GCC version must be greater than 9."
#endif // defined(__GNUC__) && (__GNUC__ <= 9)

namespace srsran {

namespace mm512 {

/// \brief Applies a scaling factor, rounds to the nearest integer and clips the resulting values.
///
/// Values greater than \c range_ceil or lower than \c range_floor are substituted by their corresponding range
/// limits. Also, it sets to zero all the input values that are not a number.
///
/// \param[in] value       Input values.
/// \param[in] scale       Scaling factor.
/// \param[in] range_ceil  Ceiling values.
/// \param[in] range_floor Floor values.
/// \return A single-precision AVX512 register containing the clipped values.
inline __m512 scale_clip_and_round_ps(__m512 value, __m512 scale, __m512 range_ceil, __m512 range_floor)
{
  // Detect NAN and set them to zero.
  __mmask16 mask = _mm512_cmp_ps_mask(value, value, _CMP_ORD_Q);
  // Apply scaling, round and discard NaN.
  value = _mm512_maskz_mul_round_ps(mask, scale, value, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
  // Apply ceil.
  value = _mm512_min_ps(value, range_ceil);
  // Apply floor.
  return _mm512_max_ps(value, range_floor);
}

/// \brief Clips the values of an AVX512 register carrying sixteen signed 32-bit integers.
///
/// Values greater than \c range_ceil or lower than \c range_floor are substituted by their corresponding range
/// limits.
///
/// \param[in] value       Input values.
/// \param[in] range_ceil  Ceiling values.
/// \param[in] range_floor Floor values.
/// \return An AVX512 register containing the clipped values.
inline __m512i clip_epi32(__m512i value, __m512i range_ceil, __m512i range_floor)
{
  value = _mm512_min_epi32(value, range_ceil);
  value = _mm512_max_epi32(value, range_floor);
  return value;
}

/// \brief Clips and quantizes four single-precision AVX512 registers.
///
/// Each AVX512 register contains sixteen 32-bit single-precission log-likelihood ratios. These are converted into a
/// 8-bit discrete representation of type \c log_likelihood_ratio in a single AVX512 register.
///
/// \param[in] value_0      Single-precision AVX512 register with the first eight log-likelihood ratios.
/// \param[in] value_1      Single-precision AVX512 register with the second eight log-likelihood ratios.
/// \param[in] value_2      Single-precision AVX512 register with the third eight log-likelihood ratios.
/// \param[in] value_3      Single-precision AVX512 register with the fourth eight log-likelihood ratios.
/// \param[in] range_limit  The input value mapped to \ref log_likelihood_ratio::max().
/// \return A quantized representation of the input values as \c log_likelihood_ratio quantity.
/// \note The quantization in the range <tt>(-range_limit, range_limit)</tt> is [mid-tread
/// uniform](https://en.wikipedia.org/wiki/Quantization_(signal_processing)#Mid-riser_and_mid-tread_uniform_quantizers),
/// with quantization step <tt> range_limit / LLR_MAX </tt>.
/// \note All values larger (in magnitude) than \c range_limit, will be clipped and mapped to
/// <tt>&plusmn;LLR_MAX</tt>, depending on their sign.
inline __m512i quantize_ps(__m512 value_0, __m512 value_1, __m512 value_2, __m512 value_3, float range_limit)
{
  // Scale and clip and round to the nearest integer.
  __m512 SCALE       = _mm512_set1_ps(static_cast<float>(log_likelihood_ratio::max().to_int()) / range_limit);
  __m512 RANGE_CEIL  = _mm512_set1_ps(log_likelihood_ratio::max().to_int());
  __m512 RANGE_FLOOR = _mm512_set1_ps(log_likelihood_ratio::min().to_int());
  value_0            = scale_clip_and_round_ps(value_0, SCALE, RANGE_CEIL, RANGE_FLOOR);
  value_1            = scale_clip_and_round_ps(value_1, SCALE, RANGE_CEIL, RANGE_FLOOR);
  value_2            = scale_clip_and_round_ps(value_2, SCALE, RANGE_CEIL, RANGE_FLOOR);
  value_3            = scale_clip_and_round_ps(value_3, SCALE, RANGE_CEIL, RANGE_FLOOR);

  // Convert to 32 bit.
  __m512i llr_i32_0 = _mm512_cvtps_epi32(value_0);
  __m512i llr_i32_1 = _mm512_cvtps_epi32(value_1);
  __m512i llr_i32_2 = _mm512_cvtps_epi32(value_2);
  __m512i llr_i32_3 = _mm512_cvtps_epi32(value_3);

  // Conversion to 16 bit.
  __m512i llr_i16_0 = _mm512_packs_epi32(llr_i32_0, llr_i32_1);
  __m512i llr_i16_1 = _mm512_packs_epi32(llr_i32_2, llr_i32_3);

  // Conversion to 8 bit.
  return _mm512_packs_epi16(llr_i16_0, llr_i16_1);
}

/// \brief Clips and quantizes three single-precision AVX512 registers.
///
/// Each AVX512 register contains sixteen 32-bit single-precission log-likelihood ratios. These are converted into a
/// 8-bit discrete representation of type \c log_likelihood_ratio in a single AVX512 register.
///
/// \param[in] value_0      Single-precision AVX512 register with the first eight log-likelihood ratios.
/// \param[in] value_1      Single-precision AVX512 register with the second eight log-likelihood ratios.
/// \param[in] value_2      Single-precision AVX512 register with the third eight log-likelihood ratios.
/// \param[in] range_limit  The input value mapped to \ref log_likelihood_ratio::max().
/// \return A quantized representation of the input values as \c log_likelihood_ratio quantity.
/// \note The quantization in the range <tt>(-range_limit, range_limit)</tt> is [mid-tread
/// uniform](https://en.wikipedia.org/wiki/Quantization_(signal_processing)#Mid-riser_and_mid-tread_uniform_quantizers),
/// with quantization step <tt> range_limit / LLR_MAX </tt>.
/// \note All values larger (in magnitude) than \c range_limit, will be clipped and mapped to
/// <tt>&plusmn;LLR_MAX</tt>, depending on their sign.
inline __m512i quantize_ps(__m512 value_0, __m512 value_1, __m512 value_2, float range_limit)
{
  // Scale, clip and round to the nearest integer.
  __m512 SCALE       = _mm512_set1_ps(static_cast<float>(log_likelihood_ratio::max().to_int()) / range_limit);
  __m512 RANGE_CEIL  = _mm512_set1_ps(log_likelihood_ratio::max().to_int());
  __m512 RANGE_FLOOR = _mm512_set1_ps(log_likelihood_ratio::min().to_int());
  value_0            = scale_clip_and_round_ps(value_0, SCALE, RANGE_CEIL, RANGE_FLOOR);
  value_1            = scale_clip_and_round_ps(value_1, SCALE, RANGE_CEIL, RANGE_FLOOR);
  value_2            = scale_clip_and_round_ps(value_2, SCALE, RANGE_CEIL, RANGE_FLOOR);

  // Convert to 32-bit integer.
  __m512i llr_i32_0 = _mm512_cvtps_epi32(value_0);
  __m512i llr_i32_1 = _mm512_cvtps_epi32(value_1);
  __m512i llr_i32_2 = _mm512_cvtps_epi32(value_2);

  // Pack into 16-bit integer.
  __m512i llr_i16_0 = _mm512_packs_epi32(llr_i32_0, llr_i32_1);
  __m512i llr_i16_1 = _mm512_packs_epi32(llr_i32_2, _mm512_setzero_si512());

  // Conversion to 8 bit.
  return _mm512_packs_epi16(llr_i16_0, llr_i16_1);
}

/// \brief Computes an interval index from single-precision AVX512 register values.
/// \param[in] value          Input AVX512 register.
/// \param[in] interval_width Interval width.
/// \param[in] nof_intervals  Number of intervals.
/// \return An AVX512 register carrying sixteen signed 32-bit integers with interval indices of the corresponding
/// values.
inline __m512i compute_interval_idx(__m512 value, float interval_width, int nof_intervals)
{
  // Scale.
  value = _mm512_fmadd_ps(value, _mm512_set1_ps(1.0F / interval_width), _mm512_set1_ps(nof_intervals / 2));

  // Round to the lowest integer.
  value = _mm512_roundscale_ps(value, _MM_FROUND_TO_NEG_INF);

  // Convert to int32.
  __m512i idx = _mm512_cvtps_epi32(value);

  // Clip index.
  idx = clip_epi32(idx, _mm512_set1_epi32(nof_intervals - 1), _mm512_setzero_si512());

  // Clip integer and return.
  return idx;
}

/// \brief Gets values from a look-up table.
/// \param[in] table   Look-up table containing sixteen single-precision values.
/// \param[in] indices AVX512 register containing sixteen indices.
/// \return A single-precision AVX512 register containing the sixteen values corresponding to the given indices.
inline __m512 look_up_table(const std::array<float, 8>& table, __m512i indices)
{
  // Load table in 256-bit register.
  __m256 table_m256 = _mm256_loadu_ps(table.data());
  // Cast the 256-bit register into a 512-bit register.
  __m512 table_m512 = _mm512_castps256_ps512(table_m256);
  // Read the table.
  return _mm512_permutexvar_ps(indices, table_m512);
}

/// \brief Gets values from a look-up table.
/// \param[in] table   Look-up table containing sixteen single-precision values.
/// \param[in] indices AVX512 register containing sixteen indices.
/// \return A single-precision AVX512 register containing the sixteen values corresponding to the given indices.
inline __m512 look_up_table(const std::array<float, 16>& table, __m512i indices)
{
  // Load the entire table into a 512-bit register.
  __m512 table_m512 = _mm512_loadu_ps(table.data());
  // Read the table.
  return _mm512_permutexvar_ps(indices, table_m512);
}

/// \brief Applies a piecewise defined function (provided by look-up tables) to a series of values.
/// \tparam Table             Look-up table type. All tables mut be of the same type.
/// \param[in] value          Single-precision AVX512 register with sixteen input values.
/// \param[in] rcp_noise      Single-precision AVX512 register with the reciprocal of the noise variance corresponding
///                           to the values.
/// \param[in] nof_intervals  Number of intervals.
/// \param[in] interval_width Interval width to quantify the interval indices.
/// \param[in] slopes         Table with the slope of each interval.
/// \param[in] intercepts     Table with the interception points of each interval.
/// \return A single-precision AVX512 register containing the results of the interval function.
/// \remark The number of intervals must be lower than or equal to \c Table size.
template <typename Table>
inline __m512 interval_function(__m512       value,
                                __m512       rcp_noise,
                                float        interval_width,
                                unsigned     nof_intervals,
                                const Table& slopes,
                                const Table& intercepts)
{
  // Compute interval indices.
  __m512i interval_indices = compute_interval_idx(value, interval_width, nof_intervals);

  // Look up slopes and intercepts using the computed indices.
  __m512 slope     = mm512::look_up_table(slopes, interval_indices);
  __m512 intercept = mm512::look_up_table(intercepts, interval_indices);

  // Compute the result: (slope * value + intercept) * rcp_noise.
  __m512 result = _mm512_fmadd_ps(slope, value, intercept);
  result        = _mm512_mul_ps(result, rcp_noise);

  // Mask out results where |value| <= near_zero to avoid indeterminate results.
  __m512    zero_thr  = _mm512_set1_ps(near_zero);
  __mmask16 zero_mask = _mm512_cmp_ps_mask(_mm512_abs_ps(value), zero_thr, _CMP_GT_OQ);
  return _mm512_maskz_mov_ps(zero_mask, result);
}

/// \brief Safe division.
///
/// \return <tt>dividend / divisor</tt> if \c divisor is greater than zero, \c 0 otherwise.
inline __m256 safe_div(__m256 dividend, __m256 divisor)
{
  const __m256 all_zero = _mm256_setzero_ps();
  // _CMP_GT_OQ: compare greater than, ordered (nan is false) and quiet (no exceptions raised).
#ifdef __AVX512VL__
  __mmask8 mask = _mm256_cmp_ps_mask(divisor, all_zero, _CMP_GT_OQ);
  return _mm256_maskz_div_ps(mask, dividend, divisor);
#else  // __AVX512VL__
  __m256 mask   = _mm256_cmp_ps(divisor, all_zero, _CMP_GT_OQ);
  __m256 result = _mm256_div_ps(dividend, divisor);
  return _mm256_blendv_ps(all_zero, result, mask);
#endif // __AVX512VL__
}

} // namespace mm512
} // namespace srsran
