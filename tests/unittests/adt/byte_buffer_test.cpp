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

#include "srsran/adt/byte_buffer.h"
#include "srsran/adt/detail/byte_buffer_segment_pool.h"
#include "srsran/support/executors/unique_thread.h"
#include "srsran/support/test_utils.h"
#include <condition_variable>
#include <gtest/gtest.h>
#include <list>

using namespace srsran;

static_assert(std::is_same<byte_buffer_view::value_type, uint8_t>::value, "Invalid valid_type");
static_assert(std::is_same<byte_buffer_view::iterator::value_type, uint8_t>::value, "Invalid valid_type");
static_assert(std::is_same<byte_buffer_view::const_iterator::value_type, uint8_t>::value, "Invalid valid_type");
static_assert(std::is_same<byte_buffer_view::iterator::reference, uint8_t&>::value, "Invalid reference type");
static_assert(std::is_same<byte_buffer_view::const_iterator::reference, const uint8_t&>::value,
              "Invalid reference type");
static_assert(std::is_same<byte_buffer_view::const_iterator::pointer, const uint8_t*>::value, "Invalid pointer type");
static_assert(is_byte_buffer_range<byte_buffer_view>::value, "Invalid metafunction is_byte_buffer_range");

static_assert(std::is_same<byte_buffer::value_type, uint8_t>::value, "Invalid valid_type");
static_assert(std::is_same<byte_buffer::iterator::value_type, uint8_t>::value, "Invalid valid_type");
static_assert(is_byte_buffer_range<byte_buffer>::value, "Invalid metafunction is_byte_buffer_range");

static_assert(is_byte_buffer_range<byte_buffer_slice>::value, "Invalid metafunction is_byte_buffer_range");

// Performs several checks that ensure the valid state of the byte_buffer length.
// Note1: the byte_buffer::iterator operator- sums the lengths of the byte_buffer individual segments.
// Note2: std::distance counts the number of iterations between begin() and end().
#define ASSERT_EQ_LEN(buffer, len)                                                                                     \
  ASSERT_EQ(buffer.empty(), (len) == 0);                                                                               \
  ASSERT_EQ(buffer.length(), (len));                                                                                   \
  ASSERT_EQ(buffer.end() - buffer.begin(), (len));                                                                     \
  ASSERT_EQ(std::distance(buffer.begin(), buffer.end()), (len));

// Ensures commutativity of byte_buffer::operator==, and consistency when compared to std::equal(...).
#define ASSERT_EQ_BUFFER(buffer1, buffer2)                                                                             \
  ASSERT_EQ(buffer1, buffer2);                                                                                         \
  ASSERT_TRUE(std::equal(buffer1.begin(), buffer1.end(), buffer2.begin(), buffer2.end()))                              \
      << fmt::format("\nbuf1: {}\nbuf2: {}", buffer1, buffer2);                                                        \
  ASSERT_EQ(buffer2, buffer1)

namespace {

const size_t memory_block_size = detail::get_default_byte_buffer_segment_pool().memory_block_size();

const size_t  small_vec_size = 6;
const size_t  large_vec_size = memory_block_size * 4;
static size_t random_vec_size(unsigned lb = 1, unsigned ub = large_vec_size)
{
  return test_rgen::uniform_int<unsigned>(lb, ub);
}

static std::vector<uint8_t> concat_vec(span<const uint8_t> before, span<const uint8_t> after)
{
  std::vector<uint8_t> ret(before.size() + after.size());
  std::copy(before.begin(), before.end(), ret.begin());
  std::copy(after.begin(), after.end(), ret.begin() + before.size());
  return ret;
}

void check_all_segments_have_been_destroyed()
{
  auto&    pool           = detail::get_default_byte_buffer_segment_pool();
  unsigned blocks_in_pool = pool.get_local_cache_size() + pool.get_central_cache_approx_size();
  report_fatal_error_if_not(blocks_in_pool == pool.nof_memory_blocks(),
                            "Failed to deallocate all blocks. Total blocks={}, central cache={}, local cache={}",
                            pool.nof_memory_blocks(),
                            pool.get_central_cache_approx_size(),
                            pool.get_local_cache_size());
}

/// Basic byte_buffer test that takes no parameters.
class byte_buffer_tester : public ::testing::Test
{
public:
  void TearDown() override { check_all_segments_have_been_destroyed(); }
};

/// Test fixture for tests involving a single array of bytes.
class one_vector_size_param_test : public ::testing::TestWithParam<size_t>
{
protected:
  void TearDown() override { check_all_segments_have_been_destroyed(); }

  size_t               sz1   = GetParam();
  std::vector<uint8_t> bytes = test_rgen::random_vector<uint8_t>(sz1);
};

class two_vector_size_param_test : public ::testing::TestWithParam<std::tuple<size_t, size_t>>
{
protected:
  void TearDown() override { check_all_segments_have_been_destroyed(); }

  size_t               sz1    = std::get<0>(GetParam());
  size_t               sz2    = std::get<1>(GetParam());
  std::vector<uint8_t> bytes1 = test_rgen::random_vector<uint8_t>(sz1);
  std::vector<uint8_t> bytes2 = test_rgen::random_vector<uint8_t>(sz2);
};

class three_vector_size_param_test : public ::testing::TestWithParam<std::tuple<size_t, size_t, size_t>>
{
protected:
  void TearDown() override { check_all_segments_have_been_destroyed(); }

  size_t               sz1    = std::get<0>(GetParam());
  size_t               sz2    = std::get<1>(GetParam());
  size_t               sz3    = std::get<2>(GetParam());
  std::vector<uint8_t> bytes1 = test_rgen::random_vector<uint8_t>(sz1);
  std::vector<uint8_t> bytes2 = test_rgen::random_vector<uint8_t>(sz2);
  std::vector<uint8_t> bytes3 = test_rgen::random_vector<uint8_t>(sz3);
};

/// Test fixture for tests with a probability parameter.
class byte_buffer_stress_tester : public ::testing::TestWithParam<float>
{
protected:
  void TearDown() override { check_all_segments_have_been_destroyed(); }

  float P_alloc = GetParam();
};

/// Basic byte_buffer_view test that takes no parameters.
class byte_buffer_view_tester : public ::testing::Test
{
public:
  void TearDown() override { check_all_segments_have_been_destroyed(); }
};

/// Basic byte_buffer_slice test that takes no parameters.
class byte_buffer_slice_tester : public ::testing::Test
{
public:
  void TearDown() override { check_all_segments_have_been_destroyed(); }
};

/// Basic byte_buffer_reader test that takes no parameters.
class byte_buffer_reader_tester : public ::testing::Test
{
public:
  void TearDown() override { check_all_segments_have_been_destroyed(); }
};

/// Basic byte_buffer_writer test that takes no parameters.
class byte_buffer_writer_tester : public ::testing::Test
{
public:
  void TearDown() override { check_all_segments_have_been_destroyed(); }
};

} // namespace

///////////////////////// byte_buffer_test //////////////////////////////

TEST_F(byte_buffer_tester, empty_byte_buffer_in_valid_state)
{
  byte_buffer pdu;
  ASSERT_EQ_LEN(pdu, 0);
  ASSERT_EQ(pdu, std::vector<uint8_t>{});
  ASSERT_EQ(pdu, std::list<uint8_t>{}) << "Comparison with empty non-span type failed";
  ASSERT_EQ(pdu.segments().begin(), pdu.segments().end());
  ASSERT_TRUE(pdu.is_contiguous());
  ASSERT_TRUE(pdu.resize(0));
  ASSERT_EQ_LEN(pdu, 0);
  pdu.clear();
  ASSERT_EQ_LEN(pdu, 0);
  ASSERT_EQ(pdu, pdu.deep_copy().value());
  ASSERT_EQ(pdu, pdu.copy());
  ASSERT_TRUE(pdu.append(std::vector<uint8_t>{}));
  ASSERT_EQ_LEN(pdu, 0);
  ASSERT_EQ(pdu.segments().begin(), pdu.segments().end());
}

TEST_P(one_vector_size_param_test, ctor_with_span)
{
  byte_buffer pdu = byte_buffer::create(this->bytes).value();

  ASSERT_EQ_LEN(pdu, bytes.size());
  ASSERT_TRUE(std::equal(pdu.begin(), pdu.end(), bytes.begin(), bytes.end()));
}

TEST_P(one_vector_size_param_test, equality_comparison)
{
  byte_buffer        pdu  = byte_buffer::create(this->bytes).value();
  byte_buffer        pdu2 = byte_buffer::create(this->bytes).value();
  std::list<uint8_t> not_a_span{this->bytes.begin(), this->bytes.end()};

  // comparison byte_buffer vs span.
  ASSERT_EQ_BUFFER(pdu, bytes);

  // comparison byte_buffer vs byte_buffer.
  ASSERT_EQ_BUFFER(pdu, pdu2);

  // comparison byte_buffer vs any other range type.
  ASSERT_EQ(pdu, not_a_span);

  // comparison byte_buffer vs other range of larger length.
  std::vector<uint8_t> larger_bytes = concat_vec(bytes, test_rgen::random_vector<uint8_t>(random_vec_size()));
  std::list<uint8_t>   larger_not_a_span{larger_bytes.begin(), larger_bytes.end()};
  pdu2 = byte_buffer::create(larger_bytes).value();
  ASSERT_NE(pdu, larger_bytes);
  ASSERT_NE(larger_bytes, pdu);
  ASSERT_NE(pdu, pdu2);
  ASSERT_NE(pdu2, pdu);
  ASSERT_NE(pdu, larger_not_a_span);

  // comparison byte_buffer vs other range of shorter length.
  std::vector<uint8_t> shorter_bytes(bytes.begin(),
                                     bytes.begin() + test_rgen::uniform_int<unsigned>(0, bytes.size() - 1));
  std::list<uint8_t>   shorter_not_a_span{shorter_bytes.begin(), shorter_bytes.end()};
  pdu2 = byte_buffer::create(shorter_bytes).value();
  ASSERT_NE(pdu, shorter_bytes);
  ASSERT_NE(shorter_bytes, pdu);
  ASSERT_NE(pdu, pdu2);
  ASSERT_NE(pdu2, pdu);
  ASSERT_NE(pdu, shorter_not_a_span);

  // comparison of byte_buffer vs other range of equal length but different content.
  std::vector<uint8_t> neq_bytes = bytes;
  neq_bytes[test_rgen::uniform_int<unsigned>(0, neq_bytes.size() - 1)]++;
  std::list<uint8_t> neq_not_a_span{neq_bytes.begin(), neq_bytes.end()};
  ASSERT_NE(pdu, neq_bytes);
  ASSERT_NE(neq_bytes, pdu);
  ASSERT_NE(pdu, pdu2);
  ASSERT_NE(pdu2, pdu);
  ASSERT_NE(pdu, neq_not_a_span);
}

TEST_P(one_vector_size_param_test, move_ctor)
{
  byte_buffer pdu;
  ASSERT_TRUE(pdu.append(this->bytes));

  byte_buffer pdu2{std::move(pdu)};
  ASSERT_TRUE(pdu.empty());
  ASSERT_FALSE(pdu2.empty());
  ASSERT_EQ_BUFFER(pdu2, bytes);
}

TEST_F(byte_buffer_tester, initializer_list)
{
  byte_buffer          pdu = byte_buffer::create({1, 2, 3, 4, 5, 6}).value();
  std::vector<uint8_t> bytes{1, 2, 3, 4, 5, 6};

  ASSERT_EQ_LEN(pdu, 6);
  ASSERT_TRUE(std::equal(pdu.begin(), pdu.end(), bytes.begin(), bytes.end()));
}

TEST_P(two_vector_size_param_test, append)
{
  byte_buffer pdu;

  // Append span of bytes (that may occupy more than one segment).
  ASSERT_TRUE(pdu.append(this->bytes1));
  ASSERT_EQ_LEN(pdu, bytes1.size());
  ASSERT_EQ_BUFFER(pdu, bytes1);

  // Append two byte_buffers.
  byte_buffer pdu2 = byte_buffer::create(bytes2).value();
  ASSERT_EQ(pdu2, bytes2);
  ASSERT_TRUE(pdu2.append(pdu));
  ASSERT_EQ(pdu.length() + bytes2.size(), pdu2.length());
  ASSERT_EQ(pdu2.length(), pdu2.end() - pdu2.begin());
  ASSERT_EQ(pdu2, concat_vec(bytes2, bytes1));
}

TEST_P(two_vector_size_param_test, prepend)
{
  byte_buffer pdu;

  // prepend in empty byte_buffer.
  ASSERT_TRUE(pdu.prepend(bytes1));
  ASSERT_EQ(pdu.length(), bytes1.size());
  ASSERT_EQ(pdu, bytes1);

  // prepend in non-empty byte_buffer.
  ASSERT_TRUE(pdu.prepend(bytes2));
  ASSERT_EQ(pdu.length(), bytes1.size() + bytes2.size());
  ASSERT_EQ(pdu, concat_vec(bytes2, bytes1));
}

TEST_P(one_vector_size_param_test, clear)
{
  byte_buffer pdu;
  ASSERT_TRUE(pdu.append(this->bytes));

  ASSERT_TRUE(not pdu.empty());
  pdu.clear();
  ASSERT_EQ_LEN(pdu, 0);

  // multiple clear calls are valid.
  pdu.clear();
  ASSERT_TRUE(pdu.empty());
}

TEST_F(byte_buffer_tester, iterator)
{
  byte_buffer pdu;

  std::vector<uint8_t> bytes = test_rgen::random_vector<uint8_t>(small_vec_size);
  ASSERT_TRUE(pdu.append(bytes));

  // iterator
  size_t i = 0;
  for (byte_buffer::iterator it = pdu.begin(); it != pdu.end(); ++it, ++i) {
    ASSERT_EQ(bytes[i], *it);
  }
  ASSERT_EQ(bytes.size(), i);

  // const iterator
  i = 0;
  for (byte_buffer::const_iterator it = pdu.cbegin(); it != pdu.cend(); ++it, ++i) {
    ASSERT_EQ(bytes[i], *it);
  }
  ASSERT_EQ(bytes.size(), i);

  // distance
  ASSERT_EQ(bytes.size(), pdu.length());
  ASSERT_EQ(bytes.size(), (size_t)(pdu.end() - pdu.begin()));
  ASSERT_EQ(bytes.size() - 2, (size_t)(pdu.end() - (pdu.begin() + 2)));

  // multiple segments
  std::vector<uint8_t> bytes2 = test_rgen::random_vector<uint8_t>(random_vec_size());
  ASSERT_TRUE(pdu.append(bytes2));
  std::vector<uint8_t> bytes_concat = concat_vec(bytes, bytes2);

  // iterator
  i = 0;
  for (byte_buffer::iterator it = pdu.begin(); it != pdu.end(); ++it, ++i) {
    ASSERT_EQ(bytes_concat[i], *it);
  }
  ASSERT_EQ(bytes_concat.size(), i);

  // const iterator
  i = 0;
  for (byte_buffer::const_iterator it = pdu.cbegin(); it != pdu.cend(); ++it, ++i) {
    ASSERT_EQ(bytes_concat[i], *it);
  }
  ASSERT_EQ(bytes_concat.size(), i);

  // distance
  ASSERT_EQ(bytes_concat.size(), pdu.length());
  ASSERT_EQ(bytes_concat.size(), (size_t)(pdu.end() - pdu.begin()));
  ASSERT_EQ(bytes_concat.size() - 2, (size_t)(pdu.end() - (pdu.begin() + 2)));
}

TEST_F(byte_buffer_tester, deep_copy_for_empty_byte_buffer)
{
  byte_buffer pdu;
  byte_buffer pdu2;
  pdu2 = pdu.deep_copy().value();
  ASSERT_TRUE(pdu.empty());
  ASSERT_TRUE(pdu.empty());
}

TEST_P(two_vector_size_param_test, deep_copy_for_non_empty_byte_buffer)
{
  byte_buffer pdu;
  ASSERT_TRUE(pdu.append(bytes1));
  auto bytes_concat = concat_vec(bytes1, bytes2);

  // Deep copy.
  byte_buffer pdu2;
  pdu2 = pdu.deep_copy().value();
  ASSERT_FALSE(pdu.empty());
  ASSERT_FALSE(pdu2.empty());
  ASSERT_EQ_BUFFER(pdu, pdu2);
  ASSERT_EQ_BUFFER(pdu2, bytes1);

  ASSERT_TRUE(pdu2.append(bytes2));
  ASSERT_EQ_BUFFER(pdu, bytes1);
  ASSERT_NE(pdu, pdu2);
  ASSERT_EQ_BUFFER(pdu2, bytes_concat);
}

TEST_P(two_vector_size_param_test, shallow_copy_and_append)
{
  byte_buffer pdu;
  ASSERT_TRUE(pdu.append(bytes1));

  {
    byte_buffer pdu2 = pdu.copy();
    ASSERT_EQ_BUFFER(pdu2, pdu);
    ASSERT_EQ_BUFFER(pdu2, bytes1);
    ASSERT_TRUE(pdu2.append(bytes2));
    ASSERT_EQ(pdu2, pdu);
    ASSERT_EQ(pdu.length(), pdu.end() - pdu.begin()) << "shallow copied-from byte_buffer::length() got corrupted";
    ASSERT_EQ(pdu2.length(), pdu2.end() - pdu2.begin()) << "shallow copy byte_buffer::length() got corrupted";
  }
  auto combined = concat_vec(bytes1, bytes2);
  ASSERT_EQ_BUFFER(pdu, combined);
  ASSERT_EQ(pdu.length(), pdu.end() - pdu.begin());
}

TEST_P(three_vector_size_param_test, shallow_copy_prepend_and_append)
{
  byte_buffer pdu;
  ASSERT_TRUE(pdu.append(bytes1));

  {
    byte_buffer pdu2 = pdu.copy();
    ASSERT_EQ_BUFFER(pdu2, pdu);
    ASSERT_EQ_BUFFER(pdu2, bytes1);
    ASSERT_TRUE(pdu2.prepend(bytes2));
    ASSERT_EQ(pdu2, pdu);
    ASSERT_TRUE(pdu2.append(bytes3));
    ASSERT_EQ(pdu2, pdu);
    ASSERT_EQ(pdu.length(), pdu.end() - pdu.begin()) << "shallow copied-from byte_buffer::length() got corrupted";
    ASSERT_EQ(pdu2.length(), pdu2.end() - pdu2.begin()) << "shallow copy byte_buffer::length() got corrupted";
  }
  auto combined = concat_vec(bytes2, concat_vec(bytes1, bytes3));
  ASSERT_EQ_BUFFER(pdu, combined);
  ASSERT_EQ(pdu.length(), pdu.end() - pdu.begin());
}

TEST_F(byte_buffer_tester, formatter)
{
  byte_buffer          pdu;
  std::vector<uint8_t> bytes = {1, 2, 3, 4, 15, 16, 255};
  ASSERT_TRUE(pdu.append(bytes));

  fmt::print("PDU: {}\n", pdu);
  std::string result = fmt::format("{}", pdu);
  ASSERT_EQ(result, "01 02 03 04 0f 10 ff");
}

TEST_F(byte_buffer_tester, trim)
{
  byte_buffer          pdu;
  std::vector<uint8_t> bytes = test_rgen::random_vector<uint8_t>(small_vec_size);
  ASSERT_TRUE(pdu.append(bytes));

  ASSERT_EQ(pdu, bytes);
  pdu.trim_head(2);
  ASSERT_EQ(pdu, span<const uint8_t>{bytes}.subspan(2, bytes.size() - 2));

  std::vector<uint8_t> bytes2 = test_rgen::random_vector<uint8_t>(random_vec_size(2));
  ASSERT_TRUE(pdu.append(bytes2));

  auto bytes_concat = concat_vec(bytes, bytes2);
  ASSERT_EQ(pdu, span<const uint8_t>{bytes_concat}.subspan(2, bytes_concat.size() - 2));

  pdu.trim_head(bytes.size() - 2);
  ASSERT_EQ(pdu, bytes2);

  // should make the first segment empty and remove it.
  pdu.trim_head(bytes2.size() - 2);
  ASSERT_EQ(pdu, span<const uint8_t>{bytes2}.last(2));
}

TEST_F(byte_buffer_tester, prepend_and_trim_tail)
{
  byte_buffer        pdu;
  byte_buffer        sdu;
  uint32_t           pdu_len    = memory_block_size - 5 + test_rgen::uniform_int<unsigned>(0, 10);
  constexpr uint32_t trim_len   = 4;
  constexpr uint32_t prefix_len = 3;
  for (uint32_t i = 0; i < pdu_len; i++) {
    ASSERT_TRUE(pdu.append(i));
  }

  ASSERT_TRUE(sdu.append(pdu.begin() + prefix_len, pdu.end()));
  std::array<uint8_t, prefix_len> hdr_buf;
  std::copy(pdu.begin(), pdu.begin() + prefix_len, hdr_buf.begin());
  ASSERT_TRUE(sdu.prepend(hdr_buf));

  ASSERT_EQ(sdu.length(), pdu_len);
  ASSERT_EQ(std::distance(sdu.begin(), sdu.end()), pdu_len);

  sdu.trim_tail(trim_len);
  ASSERT_EQ_LEN(sdu, pdu_len - trim_len);
}

TEST_P(three_vector_size_param_test, shallow_copy_prepend_and_append_keeps_validity)
{
  // When a byte_buffer::prepend causes the byte_buffer head segment to move, any previously existing shallow copies
  // could become invalidated. To avoid this issue, we perform COW on prepend, when more than one byte_buffer points to
  // the same head segment.
  byte_buffer pdu = byte_buffer::create(bytes1).value();

  byte_buffer pdu2{pdu.copy()};
  ASSERT_TRUE(pdu.prepend(bytes2));
  ASSERT_TRUE(pdu.append(bytes3));

  ASSERT_EQ(pdu, concat_vec(concat_vec(bytes2, bytes1), bytes3));
  ASSERT_EQ(pdu2.length(), pdu2.end() - pdu2.begin()) << "shallow copied-from byte_buffer::length() got corrupted";
}

TEST_P(three_vector_size_param_test, shallow_copy_reserve_prepend_and_append_keeps_validity)
{
  // When a byte_buffer::prepend causes the byte_buffer head segment to move, any previously existing shallow copies
  // could become invalidated. To avoid this issue, we perform COW on prepend, when more than one byte_buffer point to
  // the same head segment.
  byte_buffer pdu = byte_buffer::create(bytes1).value();

  byte_buffer      pdu2{pdu.copy()};
  byte_buffer_view v = pdu.reserve_prepend(bytes2.size());
  ASSERT_TRUE(pdu.append(bytes3));
  std::copy(bytes2.begin(), bytes2.end(), v.begin());

  ASSERT_EQ(pdu, concat_vec(concat_vec(bytes2, bytes1), bytes3));
  ASSERT_EQ(pdu2.length(), pdu2.end() - pdu2.begin()) << "shallow copied-from byte_buffer::length() got corrupted";
}

TEST_F(byte_buffer_tester, is_contiguous)
{
  byte_buffer          pdu;
  std::vector<uint8_t> bytes        = test_rgen::random_vector<uint8_t>(small_vec_size);
  std::vector<uint8_t> bytes2       = test_rgen::random_vector<uint8_t>(large_vec_size);
  auto                 bytes_concat = concat_vec(bytes, bytes2);

  ASSERT_TRUE(pdu.append(bytes));
  ASSERT_TRUE(pdu.is_contiguous());
  ASSERT_TRUE(pdu.append(byte_buffer::create(bytes2).value()));
  ASSERT_TRUE(not pdu.is_contiguous());

  ASSERT_EQ_BUFFER(pdu, bytes_concat);
  ASSERT_FALSE(pdu.linearize());
  ASSERT_EQ_BUFFER(pdu, bytes_concat) << "A failed linearization should not alter the original byte_buffer";

  pdu.trim_tail(pdu.length() - detail::get_default_byte_buffer_segment_pool().memory_block_size() / 2);
  ASSERT_FALSE(pdu.is_contiguous());
  ASSERT_TRUE(pdu.linearize());
  ASSERT_TRUE(pdu.is_contiguous());
}

TEST_F(byte_buffer_tester, hexdump)
{
  std::vector<uint8_t> bytes{0x1, 0x2, 0x3, 0x4, 0x5, 0xff};
  byte_buffer          pdu = make_byte_buffer("0102030405FF").value();
  ASSERT_EQ(pdu, bytes);
}

TEST_F(byte_buffer_tester, copy_byte_buffer_to_span)
{
  byte_buffer          pdu;
  std::vector<uint8_t> bytes        = test_rgen::random_vector<uint8_t>(small_vec_size);
  std::vector<uint8_t> bytes2       = test_rgen::random_vector<uint8_t>(random_vec_size());
  auto                 bytes_concat = concat_vec(bytes, bytes2);

  std::vector<uint8_t> dst_vec(bytes_concat.size(), 0xfe);
  span<uint8_t>        dst_span = {dst_vec};
  size_t               len      = 0;

  // test copy of empty buffer
  len = copy_segments(pdu, dst_span);
  ASSERT_EQ(len, 0);
  ASSERT_EQ(pdu.length(), 0);
  ASSERT_TRUE(std::all_of(dst_span.begin(), dst_span.end(), [](uint8_t v) { return v == 0xfe; }));

  // test copy of small buffer
  ASSERT_TRUE(pdu.append(bytes));
  len = copy_segments(pdu, dst_span);
  ASSERT_EQ(len, pdu.length());
  ASSERT_TRUE(std::equal(pdu.begin(), pdu.end(), dst_span.begin(), dst_span.begin() + len));
  ASSERT_EQ(pdu, dst_span.subspan(0, len));
  ASSERT_EQ(dst_span.data()[len], 0xfe);

  // test copy of large buffer
  ASSERT_TRUE(pdu.append(bytes2));
  len = copy_segments(pdu, dst_span);
  ASSERT_EQ(len, pdu.length());
  ASSERT_EQ(dst_span, pdu);

  // test copy to short span
  std::fill(dst_span.begin(), dst_span.end(), 0xfe);
  span<uint8_t> dst_subspan = dst_span.subspan(0, pdu.length() - 1);
  len                       = copy_segments(pdu, dst_subspan);
  ASSERT_EQ(len, pdu.length() - 1);
  ASSERT_TRUE(std::equal(dst_subspan.begin(), dst_subspan.end(), pdu.begin()));
  ASSERT_EQ(dst_span.data()[len], 0xfe);
}

TEST_F(byte_buffer_tester, copy_byte_buffer_view_to_span)
{
  byte_buffer          pdu;
  std::vector<uint8_t> bytes        = test_rgen::random_vector<uint8_t>(small_vec_size);
  std::vector<uint8_t> bytes2       = test_rgen::random_vector<uint8_t>(random_vec_size());
  auto                 bytes_concat = concat_vec(bytes, bytes2);

  std::vector<uint8_t> dst_vec(bytes_concat.size(), 0xfe);
  span<uint8_t>        dst_span = {dst_vec};
  size_t               len      = 0;

  // test copy of empty buffer
  len = copy_segments(byte_buffer_view{pdu}, dst_span);
  ASSERT_EQ(len, 0);
  ASSERT_EQ(pdu.length(), 0);
  ASSERT_TRUE(std::all_of(dst_span.begin(), dst_span.end(), [](uint8_t v) { return v == 0xfe; }));

  // test copy of small buffer
  ASSERT_TRUE(pdu.append(bytes));
  len = copy_segments(byte_buffer_view{pdu}, dst_span);
  ASSERT_EQ(len, pdu.length());
  ASSERT_TRUE(std::equal(pdu.begin(), pdu.end(), dst_span.begin(), dst_span.begin() + len));
  ASSERT_EQ(pdu, dst_span.subspan(0, len));
  ASSERT_EQ(dst_span.data()[len], 0xfe);

  // test copy of large buffer
  ASSERT_TRUE(pdu.append(bytes2));
  len = copy_segments(byte_buffer_view{pdu}, dst_span);
  ASSERT_EQ(len, pdu.length());
  ASSERT_EQ(dst_span, pdu);

  // test copy to short span
  std::fill(dst_span.begin(), dst_span.end(), 0xfe);
  span<uint8_t> dst_subspan = dst_span.subspan(0, pdu.length() - 1);
  len                       = copy_segments(byte_buffer_view{pdu}, dst_subspan);
  ASSERT_EQ(len, pdu.length() - 1);
  ASSERT_TRUE(std::equal(dst_subspan.begin(), dst_subspan.end(), pdu.begin()));
  ASSERT_EQ(dst_span.data()[len], 0xfe);
}

TEST_F(byte_buffer_tester, to_span)
{
  byte_buffer          pdu;
  std::vector<uint8_t> bytes        = test_rgen::random_vector<uint8_t>(small_vec_size);
  std::vector<uint8_t> bytes2       = test_rgen::random_vector<uint8_t>(large_vec_size);
  auto                 bytes_concat = concat_vec(bytes, bytes2);

  std::vector<uint8_t> tmp_mem(bytes_concat.size(), 0);
  tmp_mem.reserve(bytes_concat.size());

  // test view of empty buffer
  span<const uint8_t> dst = to_span(pdu, tmp_mem);
  ASSERT_TRUE(dst.empty());
  ASSERT_EQ(dst, pdu);

  // test view of small buffer (no copy)
  ASSERT_TRUE(pdu.append(bytes));
  dst = to_span(pdu, tmp_mem);
  ASSERT_EQ(pdu, dst);
  ASSERT_EQ(dst, span<const uint8_t>{*pdu.segments().begin()});

  // test copy of large buffer (with copy)
  ASSERT_TRUE(pdu.append(bytes2));
  dst = to_span(pdu, tmp_mem);
  ASSERT_EQ(pdu, dst);
  ASSERT_EQ(dst, span<const uint8_t>{tmp_mem});
}

TEST_F(byte_buffer_tester, iterator_plus_equal_op)
{
  // Test with small vector of bytes
  // Make initial vector
  byte_buffer          pdu;
  std::vector<uint8_t> bytes = test_rgen::random_vector<uint8_t>(small_vec_size);
  ASSERT_TRUE(pdu.append(bytes));

  auto it = pdu.begin();
  ASSERT_NE(it, pdu.cend());

  // Point reader at 1 byte before the end
  it += (pdu.length() - 1);
  ASSERT_NE(it, pdu.cend());

  // Point reader at the end
  it = pdu.begin();
  it += pdu.length();
  ASSERT_EQ(it, pdu.cend());

  // Test with big vector of bytes
  bytes = test_rgen::random_vector<uint8_t>(large_vec_size);
  pdu.clear();
  ASSERT_TRUE(pdu.append(bytes));

  it = pdu.begin();
  ASSERT_NE(it, pdu.cend());

  // Point reader at 1 byte before the end
  it += (pdu.length() - 1);
  ASSERT_NE(pdu.cend(), it);

  // Point reader at the end
  it = pdu.begin();
  it += pdu.length();
  ASSERT_EQ(it, pdu.cend());

  // Test with byte buffer made of several segments
  ASSERT_TRUE(pdu.append(bytes));
  ASSERT_TRUE(pdu.append(bytes));

  it = pdu.begin();
  ASSERT_NE(pdu.cend(), it);

  // Point reader at 1 byte before the end
  it += (pdu.length() - 1);
  ASSERT_NE(it, pdu.cend());

  // Point reader at the end
  it = pdu.begin();
  it += pdu.length();
  ASSERT_EQ(it, pdu.cend());
}

TEST_F(byte_buffer_tester, iterator_of_segments)
{
  byte_buffer          pdu;
  std::vector<uint8_t> small_vec_bytes = test_rgen::random_vector<uint8_t>(small_vec_size);

  // empty buffer.
  ASSERT_EQ(pdu.segments().begin(), pdu.segments().end());

  // one-segment buffer
  ASSERT_TRUE(pdu.append(small_vec_bytes));
  ASSERT_NE(pdu.segments().begin(), pdu.segments().end());
  ASSERT_EQ(*pdu.segments().begin(), span<uint8_t>(small_vec_bytes));
  ASSERT_EQ(++pdu.segments().begin(), pdu.segments().end());

  // multiple-segment buffer.
  ASSERT_TRUE(pdu.append(test_rgen::random_vector<uint8_t>(random_vec_size(1, 10000))));
  std::vector<uint8_t> total_bytes(pdu.begin(), pdu.end());
  unsigned             seg_offset = 0;
  for (auto seg_it = pdu.segments().begin(); seg_it != pdu.segments().end(); ++seg_it) {
    ASSERT_TRUE(seg_it->size() > 0);
    ASSERT_EQ(*seg_it, span<uint8_t>(total_bytes.data() + seg_offset, seg_it->size()));
    seg_offset += seg_it->size();
  }
  ASSERT_EQ(seg_offset, total_bytes.size());
}

TEST_F(byte_buffer_tester, reserve_prepend)
{
  byte_buffer pdu;

  // Prepend small vector
  std::vector<uint8_t> small_vec = test_rgen::random_vector<uint8_t>(small_vec_size);
  byte_buffer_view     view      = pdu.reserve_prepend(small_vec.size());
  TESTASSERT_EQ(small_vec.size(), pdu.length());
  TESTASSERT_EQ(small_vec.size(), view.length());

  std::copy(small_vec.begin(), small_vec.end(), view.begin());
  TESTASSERT(pdu == small_vec);
  TESTASSERT(view == small_vec);

  // Prepend big vector
  std::vector<uint8_t> big_vec = test_rgen::random_vector<uint8_t>(large_vec_size);
  byte_buffer_view     view2   = pdu.reserve_prepend(big_vec.size());
  TESTASSERT_EQ(small_vec.size() + big_vec.size(), pdu.length());
  TESTASSERT_EQ(big_vec.size(), view2.length());

  std::copy(big_vec.begin(), big_vec.end(), view2.begin());
  TESTASSERT(view2 == big_vec);
}

TEST_F(byte_buffer_tester, append_rvalue_byte_buffer)
{
  byte_buffer          pdu;
  std::vector<uint8_t> big_vec       = test_rgen::random_vector<uint8_t>(large_vec_size);
  std::vector<uint8_t> small_vec     = test_rgen::random_vector<uint8_t>(small_vec_size);
  std::vector<uint8_t> bytes_concat  = concat_vec(big_vec, small_vec);
  std::vector<uint8_t> bytes_concat2 = concat_vec(bytes_concat, big_vec);

  // Chain small vector to empty buffer
  byte_buffer pdu2 = byte_buffer::create(small_vec).value();
  ASSERT_EQ(pdu2, small_vec);
  ASSERT_TRUE(pdu.prepend(std::move(pdu2)));
  ASSERT_FALSE(pdu.empty());
  ASSERT_EQ(pdu, small_vec);
  ASSERT_TRUE(pdu2.empty());

  // Chain byte_buffer before another non-empty byte_buffer.
  ASSERT_TRUE(pdu2.append(big_vec));
  ASSERT_TRUE(pdu.prepend(std::move(pdu2)));
  ASSERT_TRUE(pdu2.empty());
  ASSERT_EQ_LEN(pdu, big_vec.size() + small_vec.size());
  ASSERT_EQ(pdu, bytes_concat);

  // Chain byte_buffer after another non-empty byte_buffer.
  ASSERT_TRUE(pdu2.append(big_vec));
  ASSERT_TRUE(pdu.append(std::move(pdu2)));
  ASSERT_TRUE(pdu2.empty());
  ASSERT_EQ_LEN(pdu, big_vec.size() * 2 + small_vec.size());
  ASSERT_EQ(pdu, bytes_concat2);
}

TEST_P(byte_buffer_stress_tester, concurrent_alloc_dealloc_test)
{
  static const unsigned   MAX_COUNT   = 100000;
  const unsigned          NOF_THREADS = 4;
  std::mutex              mutex;
  std::condition_variable cvar;
  bool                    ready = false;
  std::atomic<unsigned>   threads_ready{0};
  const unsigned          max_buffer_size = memory_block_size * 16;
  std::vector<uint8_t>    randbytes       = test_rgen::random_vector<uint8_t>(max_buffer_size);

  // task to run in different threads.
  auto task = [&]() {
    std::vector<byte_buffer> allocated_buffers;
    allocated_buffers.reserve(1000);
    std::vector<unsigned> free_list;
    allocated_buffers.reserve(1000);
    std::uniform_real_distribution<float> rdist(0.0, 1.0);

    {
      threads_ready++;
      std::unique_lock<std::mutex> lock(mutex);
      cvar.wait(lock, [&ready]() { return ready; });
    }

    for (unsigned count = 0; count != MAX_COUNT; ++count) {
      bool alloc_or_dealloc = rdist(test_rgen::get()) <= this->P_alloc;
      if (alloc_or_dealloc) {
        // Allocation of new buffer.
        auto buf =
            byte_buffer::create(span<const uint8_t>(randbytes.data(), test_rgen::uniform_int(1U, max_buffer_size)));
        if (not buf.has_value()) {
          // pool is depleted.
          continue;
        }
        if (free_list.empty()) {
          allocated_buffers.push_back(std::move(buf.value()));
        } else {
          allocated_buffers[free_list.back()] = std::move(buf.value());
          free_list.pop_back();
        }
      } else {
        // Deallocation of existing buffer.
        if (allocated_buffers.empty()) {
          continue;
        }
        unsigned idx = test_rgen::uniform_int(0U, (unsigned)allocated_buffers.size() - 1);
        allocated_buffers[idx].clear();
        free_list.push_back(idx);
      }
    }
  };

  std::vector<std::unique_ptr<unique_thread>> workers;
  for (unsigned i = 0; i != NOF_THREADS; ++i) {
    workers.push_back(std::make_unique<unique_thread>(fmt::format("thread{}", i), task));
  }

  // Start running all threads at the same time.
  {
    while (threads_ready != NOF_THREADS) {
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
    std::lock_guard<std::mutex> lock(mutex);
    ready = true;
    cvar.notify_all();
  }

  for (auto& worker : workers) {
    worker->join();
  }
}

TEST_F(byte_buffer_tester, concurrent_alloc_dealloc_test)
{
  const unsigned           max_buffer_size = memory_block_size * 16;
  std::vector<uint8_t>     randbytes       = test_rgen::random_vector<uint8_t>(max_buffer_size);
  std::vector<byte_buffer> allocated_buffers;

  // Deplete the pool
  while (true) {
    auto pdu = byte_buffer::create(span<const uint8_t>{randbytes.data(), 10U});
    if (not pdu.has_value()) {
      break;
    }
    allocated_buffers.push_back(std::move(pdu.value()));
  }

  // Pool is still empty.
  auto pdu = byte_buffer::create(span<const uint8_t>{randbytes.data(), test_rgen::uniform_int(1U, max_buffer_size)});
  ASSERT_FALSE(pdu.has_value());

  // Test if a span can be added to a byte_buffer with heap as fallback allocator.
  size_t sz = test_rgen::uniform_int(1U, max_buffer_size);
  pdu       = byte_buffer{byte_buffer::fallback_allocation_tag{}, span<const uint8_t>{randbytes.data(), sz}};
  ASSERT_EQ(pdu.value().length(), sz);
  span<const uint8_t> expected_bytes{randbytes.data(), sz};
  ASSERT_EQ(pdu.value(), expected_bytes);

  // Test if a byte_buffer can be added to a byte_buffer with heap as fallback allocator.
  pdu = byte_buffer{byte_buffer::fallback_allocation_tag{}, allocated_buffers.front()};
  ASSERT_EQ(pdu.value().length(), allocated_buffers.front().length());
  ASSERT_EQ(pdu.value(), allocated_buffers.front());
}

INSTANTIATE_TEST_SUITE_P(byte_buffer_test,
                         one_vector_size_param_test,
                         ::testing::Values(small_vec_size, large_vec_size, random_vec_size()));

INSTANTIATE_TEST_SUITE_P(byte_buffer_test,
                         two_vector_size_param_test,
                         ::testing::Combine(::testing::Values(small_vec_size, large_vec_size, random_vec_size()),
                                            ::testing::Values(small_vec_size, large_vec_size, random_vec_size())));

INSTANTIATE_TEST_SUITE_P(byte_buffer_test,
                         three_vector_size_param_test,
                         ::testing::Combine(::testing::Values(small_vec_size, large_vec_size, random_vec_size()),
                                            ::testing::Values(small_vec_size, large_vec_size, random_vec_size()),
                                            ::testing::Values(small_vec_size, large_vec_size, random_vec_size())));

INSTANTIATE_TEST_SUITE_P(byte_buffer_test, byte_buffer_stress_tester, ::testing::Values(0.01, 0.1, 0.5, 0.9, 0.99));

///////////////////////// byte_buffer_view_test //////////////////////////////

TEST_F(byte_buffer_view_tester, empty_byte_buffer_view_is_in_valid_state)
{
  byte_buffer_view view;
  ASSERT_EQ_LEN(view, 0);
  ASSERT_EQ(view.begin(), view.end());
  ASSERT_EQ(view, byte_buffer_view{});
  ASSERT_EQ(view.view(0, 0), view);
  ASSERT_EQ(view.segments().begin(), view.segments().end());
  ASSERT_EQ(view, std::vector<uint8_t>{});

  byte_buffer pdu;
  view = pdu;
  ASSERT_EQ_LEN(view, 0);
}

TEST_F(byte_buffer_view_tester, length)
{
  byte_buffer          pdu;
  unsigned             len   = test_rgen::uniform_int<unsigned>(1, 100000);
  std::vector<uint8_t> bytes = test_rgen::random_vector<uint8_t>(len);
  ASSERT_TRUE(pdu.append(bytes));

  byte_buffer_view view = pdu;

  ASSERT_FALSE(view.empty());
  ASSERT_EQ(len, view.length());
  ASSERT_EQ(len, view.end() - view.begin());
  unsigned offset = test_rgen::uniform_int<unsigned>(0, len - 1);
  unsigned len2   = test_rgen::uniform_int<unsigned>(1, len - offset);
  ASSERT_EQ(len2, view.view(offset, len2).length());
}

TEST_F(byte_buffer_view_tester, segment_iterator)
{
  std::vector<uint8_t> bytes = test_rgen::random_vector<uint8_t>(random_vec_size());
  byte_buffer          pdu   = byte_buffer::create(bytes).value();

  unsigned         offset      = test_rgen::uniform_int<unsigned>(0, bytes.size() - 1);
  unsigned         last_offset = test_rgen::uniform_int<unsigned>(offset + 1, bytes.size());
  byte_buffer_view view{pdu.begin() + offset, pdu.begin() + last_offset};

  unsigned seg_offset = offset;
  for (auto seg_it = view.segments().begin(); seg_it != view.segments().end(); ++seg_it) {
    ASSERT_NE(seg_it, view.segments().end());
    unsigned seg_len = seg_it->size();
    ASSERT_GT(seg_len, 0);
    ASSERT_EQ(*seg_it, span<const uint8_t>(bytes.data() + seg_offset, seg_len));
    seg_offset += seg_len;
  }
  ASSERT_EQ(seg_offset, last_offset);
}

///////////////////////// byte_buffer_slice_test //////////////////////////////

TEST_F(byte_buffer_slice_tester, empty_slice_is_in_valid_state)
{
  byte_buffer_slice pkt;

  ASSERT_TRUE(pkt.empty());
  ASSERT_EQ(0, pkt.length());
  ASSERT_EQ(pkt.begin(), pkt.end());
}

TEST_F(byte_buffer_slice_tester, ctor_with_span)
{
  std::vector<uint8_t> vec   = test_rgen::random_vector<uint8_t>(test_rgen::uniform_int<unsigned>(1, large_vec_size));
  byte_buffer_slice    slice = byte_buffer_slice::create(vec).value();

  ASSERT_EQ_LEN(slice, vec.size());
  // Test operator[].
  for (unsigned i = 0; i < vec.size(); ++i) {
    ASSERT_EQ(vec[i], slice[i]);
  }
  // Test iterator.
  ASSERT_TRUE(std::equal(slice.begin(), slice.end(), vec.begin(), vec.end()));
  unsigned count = 0;
  for (uint8_t b : slice) {
    ASSERT_EQ(vec[count++], b);
  }
  // Test operator==.
  ASSERT_EQ(slice, vec);
}

TEST_F(byte_buffer_slice_tester, shallow_copy)
{
  std::vector<uint8_t> vec = test_rgen::random_vector<uint8_t>(test_rgen::uniform_int<unsigned>(1, large_vec_size));
  byte_buffer          pdu = byte_buffer::create(vec).value();

  byte_buffer_slice slice{pdu.copy()};

  // Test operator==.
  ASSERT_EQ_LEN(slice, vec.size());
  ASSERT_EQ(slice, pdu);
  ASSERT_EQ(pdu, slice);
  ASSERT_EQ(slice, vec);
  ASSERT_EQ(vec, slice);

  // slice gets altered because it is a shallow copy.
  pdu[0]++;
  ASSERT_EQ(pdu, slice);
  ASSERT_NE(slice, vec);

  // Test slice doesn't get altered by underlying byte_buffer extension.
  // pdu.append(1);
  // ASSERT_NE(slice.length(), pdu.length());
  // ASSERT_NE(slice, pdu);
  // byte_buffer_view v{pdu, 0, pdu.length() - 1};
  // TESTASSERT(slice == v);
  // TODO: Fix.
}

TEST_F(byte_buffer_slice_tester, deep_slice)
{
  std::vector<uint8_t> vec = test_rgen::random_vector<uint8_t>(random_vec_size());
  byte_buffer          pdu = byte_buffer::create(vec).value();

  byte_buffer_slice slice{pdu.deep_copy().value()};

  // Test operator[].
  for (unsigned i = 0; i < vec.size(); ++i) {
    ASSERT_EQ(vec[i], slice[i]);
  }

  // Test iterator.
  ASSERT_NE(slice.begin(), slice.end());
  unsigned count = 0;
  for (uint8_t v : slice) {
    ASSERT_EQ(vec[count++], v);
  }
  ASSERT_EQ(vec.size(), count);

  // Test operator==.
  ASSERT_EQ(slice, pdu);
  ASSERT_EQ(pdu, slice);
  ASSERT_EQ(slice, vec);

  // Test slice doesn't get altered by underlying byte_buffer extension.
  ASSERT_TRUE(pdu.append(1));
  ASSERT_NE(slice, pdu);
  byte_buffer_view v{pdu, 0, pdu.length() - 1};
  ASSERT_EQ(slice, v);

  // Test that slice is a shallow copy.
  *pdu.begin() = 255U;
  ASSERT_NE(slice, pdu);
  ASSERT_EQ(slice, vec);
}

TEST_F(byte_buffer_slice_tester, move_ctor)
{
  std::vector<uint8_t> vec = test_rgen::random_vector<uint8_t>(random_vec_size());
  byte_buffer          pdu = byte_buffer::create(vec).value();

  byte_buffer_slice slice{std::move(pdu)};
  ASSERT_TRUE(pdu.empty());

  // Test operator[].
  for (unsigned i = 0; i < vec.size(); ++i) {
    ASSERT_EQ(vec[i], slice[i]);
  }

  // Test iterator.
  ASSERT_NE(slice.begin(), slice.end());
  unsigned count = 0;
  for (uint8_t v : slice) {
    ASSERT_EQ(vec[count++], v);
  }
  ASSERT_EQ(vec.size(), count);

  // Test operator==.
  ASSERT_EQ(slice, vec);
}

TEST_F(byte_buffer_slice_tester, formatter)
{
  byte_buffer          pdu;
  std::vector<uint8_t> bytes = {1, 2, 3, 4, 15, 16, 255};
  ASSERT_TRUE(pdu.append(bytes));
  byte_buffer_slice slice{std::move(pdu), 1, 4};

  std::string result = fmt::format("{}", slice);
  ASSERT_EQ("02 03 04 0f", result);
}

///////////////////////// byte_buffer_reader_test //////////////////////////////

TEST_F(byte_buffer_reader_tester, split_advance)
{
  // Test with small vector of bytes
  // Make initial vector
  byte_buffer          pdu;
  std::vector<uint8_t> bytes = test_rgen::random_vector<uint8_t>(small_vec_size);
  ASSERT_TRUE(pdu.append(bytes));

  auto it = pdu.begin();
  TESTASSERT(it != pdu.cend());

  // Get reader and check that beginning and end coincide with the original
  byte_buffer_reader pdu_reader{byte_buffer_view{pdu}};
  TESTASSERT(pdu_reader.begin() == pdu.begin());
  TESTASSERT(pdu_reader.end() == pdu.end());

  // Stop reader at 1 byte to the end
  auto view = pdu_reader.split_and_advance(pdu.length() - 1);
  TESTASSERT(pdu_reader.begin() != pdu_reader.end());
  TESTASSERT(pdu_reader.end() == pdu.end());
  TESTASSERT(view.begin() == pdu.begin());
  TESTASSERT(view.end() == pdu_reader.begin());

  // Move reader 1 byte ahead, to the end
  view = pdu_reader.split_and_advance(1);
  TESTASSERT(pdu_reader.begin() == pdu_reader.end());
  TESTASSERT(view.end() == pdu_reader.begin());

  // Test with byte buffer made of several segments
  // Make initial vector
  bytes = test_rgen::random_vector<uint8_t>(large_vec_size);
  pdu.clear();
  ASSERT_TRUE(pdu.append(bytes));
  ASSERT_TRUE(pdu.append(bytes));
  ASSERT_TRUE(pdu.append(bytes));

  it = pdu.begin();
  TESTASSERT(it != pdu.cend());

  // Get reader and check that beginning and end coincide with the original
  byte_buffer_reader pdu_long_reader{byte_buffer_view{pdu}};
  TESTASSERT(pdu_long_reader.begin() == pdu.begin());
  TESTASSERT(pdu_long_reader.end() == pdu.end());

  // Stop reader at 1 byte to the end
  view = pdu_long_reader.split_and_advance(pdu.length() - 1);
  TESTASSERT(pdu_long_reader.begin() != pdu_long_reader.end());
  TESTASSERT(pdu_long_reader.end() == pdu.end());
  TESTASSERT(view.begin() == pdu.begin());
  TESTASSERT(view.end() == pdu_long_reader.begin());

  // Move reader 1 byte ahead, to the end
  view = pdu_long_reader.split_and_advance(1);
  TESTASSERT(pdu_long_reader.begin() == pdu_long_reader.end());
  TESTASSERT(view.end() == pdu_long_reader.begin());
}

TEST_F(byte_buffer_writer_tester, all)
{
  byte_buffer        pdu;
  byte_buffer_writer writer{pdu};

  TESTASSERT(pdu.empty());
  TESTASSERT(writer.empty());

  TESTASSERT(writer.append(5));
  TESTASSERT(not pdu.empty());
  TESTASSERT(not writer.empty());
  TESTASSERT_EQ(1, pdu.length());
  TESTASSERT_EQ(1, writer.length());

  ASSERT_TRUE(writer.append({0, 1, 2, 3, 4}));
  TESTASSERT_EQ(6, pdu.length());
  TESTASSERT_EQ(6, writer.length());
  bool is_eq = pdu == std::vector<uint8_t>{5, 0, 1, 2, 3, 4};
  TESTASSERT(is_eq);

  TESTASSERT_EQ(4, writer.back());
  writer.back() += 6;
  TESTASSERT_EQ(10, writer.back());
}
