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

#include "srsran/adt/byte_buffer.h"
#include "srsran/adt/span.h"
#include "srsran/srslog/srslog.h"
#include "srsran/support/compiler.h"
#include "srsran/support/srsran_assert.h"
#include <any>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

namespace asn1 {

#define ASN_16K 16384
#define ASN_64K 65536

template <class Integer>
constexpr Integer ceil_frac(Integer n, Integer d)
{
  return (n + (d - 1)) / d;
}

namespace detail {

inline constexpr std::size_t max_size_impl(std::size_t lhs, std::size_t rhs)
{
  return lhs < rhs ? rhs : lhs;
}

template <typename ItBegin, typename ItEnd>
inline constexpr std::size_t max_array_impl(ItBegin b, ItEnd e)
{
  return b + 1 == e ? *b : max_size_impl(*b, max_array_impl(b + 1, e));
}

} // namespace detail

/// \brief Compute maximum value of an array of std::size.
template <std::size_t N>
inline constexpr std::size_t max_size(const std::size_t (&arr)[N])
{
  return detail::max_array_impl(arr, arr + N);
}

/************************
        logging
************************/

template <typename... Args>
void log_error(const char* format, Args&&... args)
{
  srslog::fetch_basic_logger("ASN1").error(format, std::forward<Args>(args)...);
}

template <typename... Args>
void log_warning(const char* format, Args&&... args)
{
  srslog::fetch_basic_logger("ASN1").warning(format, std::forward<Args>(args)...);
}

template <typename... Args>
void log_info(const char* format, Args&&... args)
{
  srslog::fetch_basic_logger("ASN1").info(format, std::forward<Args>(args)...);
}

template <typename... Args>
void log_debug(const char* format, Args&&... args)
{
  srslog::fetch_basic_logger("ASN1").debug(format, std::forward<Args>(args)...);
}

void warn_assert(bool cond, const char* filename, int lineno);
void log_invalid_access_choice_id(uint32_t val, uint32_t choice_id);
void log_invalid_choice_id(uint32_t val, const char* choice_type);
void invalid_enum_number(int value, const char* name);
void assert_choice_type(uint32_t val, uint32_t choice_id);
template <typename Enumerated>
void assert_choice_type(typename Enumerated::options access_type, Enumerated& current_type, const char* choice_type)
{
  if (SRSRAN_UNLIKELY(current_type.value != access_type)) {
    log_error("Invalid field access for choice type \"{}\" (\"{}\"!=\"{}\")",
              choice_type,
              Enumerated(access_type).to_string(),
              current_type.to_string());
  }
}

/************************
     error handling
************************/

enum SRSASN_CODE {
  SRSASN_ERROR             = -3,
  SRSASN_ERROR_ENCODE_FAIL = -2,
  SRSASN_ERROR_DECODE_FAIL = -1,
  SRSASN_SUCCESS           = 0
};

void log_error_code(SRSASN_CODE code, const char* filename, int line);

#define HANDLE_CODE(ret)                                                                                               \
  do {                                                                                                                 \
    SRSASN_CODE macrocode = ((ret));                                                                                   \
    if (macrocode != SRSASN_SUCCESS) {                                                                                 \
      log_error_code(macrocode, __FILE__, __LINE__);                                                                   \
      return macrocode;                                                                                                \
    }                                                                                                                  \
  } while (0)

const char* convert_enum_idx(const char* array[], uint32_t nof_types, uint32_t enum_val, const char* enum_type);
bool        convert_enum_str(const char* array[],
                             uint32_t    nof_types,
                             const char* str,
                             uint32_t&   enum_val,
                             const char* enum_type);
template <class ItemType>
ItemType map_enum_number(ItemType* array, uint32_t nof_types, uint32_t enum_val, const char* enum_type);

struct ValOrError {
  uint32_t    val;
  SRSASN_CODE code;
  ValOrError() : val(0), code(SRSASN_SUCCESS) {}
  ValOrError(uint32_t val_, SRSASN_CODE code_) : val(val_), code(code_) {}
};

/************************
        bit_ref
************************/

// write+read bit_ref version
struct bit_ref {
  bit_ref(srsran::byte_buffer_writer byte_writer) : writer(byte_writer) {}

  srsran::byte_buffer_view data() const { return writer.view(); }

  int distance_bytes() const;
  int distance() const;

  SRSASN_CODE pack(uint64_t val, uint32_t n_bits);
  SRSASN_CODE pack_bytes(srsran::span<const uint8_t> bytes);
  SRSASN_CODE pack_bytes(srsran::byte_buffer_view bytes);
  SRSASN_CODE align_bytes_zero();

private:
  srsran::byte_buffer_writer writer;
  uint8_t                    offset = 0;
};

// read-only bit_ref
struct cbit_ref {
  cbit_ref(srsran::byte_buffer_view buffer_) : buffer(buffer_), it(buffer.begin()) {}
  cbit_ref(const bit_ref& bref) : buffer(bref.data()), it(bref.data().begin()) {}

  /// Construct a cbit_ref with an offset and max size, starting from the current cbit_ref position.
  cbit_ref subview(uint32_t offset_bytes, uint32_t len_bytes) const;

  srsran::byte_buffer_view data() const { return buffer; }

  int         distance_bytes() const;
  int         distance() const;
  int         distance(const cbit_ref& other) const;
  SRSASN_CODE advance_bits(uint32_t n_bits);
  SRSASN_CODE advance_bytes(uint32_t bytes);

  template <class T>
  SRSASN_CODE unpack(T& val, uint32_t n_bits);
  SRSASN_CODE unpack_bytes(srsran::span<uint8_t> bytes);
  SRSASN_CODE align_bytes();

private:
  srsran::byte_buffer_view            buffer;
  srsran::byte_buffer::const_iterator it;
  uint8_t                             offset = 0;
};

/*********************
  function helpers
*********************/
template <class T>
class dyn_array
{
public:
  typedef T value_type;
  using iterator       = T*;
  using const_iterator = const T*;

  dyn_array() = default;
  explicit dyn_array(uint32_t new_size) : size_(new_size), cap_(new_size)
  {
    if (size_ > 0) {
      data_ = new T[size_];
    }
  }
  dyn_array(const dyn_array<T>& other) : dyn_array(other.data(), other.size()) {}
  dyn_array(const T* ptr, uint32_t nof_items) : size_(nof_items), cap_(nof_items)
  {
    if (size_ > 0) {
      data_ = new T[cap_];
      std::copy(ptr, ptr + size_, data_);
    }
  }
  ~dyn_array() { delete[] data_; }
  uint32_t size() const { return size_; }
  uint32_t capacity() const { return cap_; }
  T&       operator[](uint32_t idx)
  {
    srsran_assert(idx < size(), "out-of-bounds access to dyn_array ({} >= {})", idx, size());
    return data_[idx];
  }
  const T& operator[](uint32_t idx) const
  {
    srsran_assert(idx < size(), "out-of-bounds access to dyn_array ({} >= {})", idx, size());
    return data_[idx];
  }
  dyn_array<T>& operator=(const dyn_array<T>& other)
  {
    if (this == &other) {
      return *this;
    }
    if (cap_ < other.size()) {
      delete[] data_;
      data_ = new T[other.size()];
      cap_  = other.size();
    }
    size_ = other.size();
    std::copy(other.data(), other.data() + other.size(), data());
    return *this;
  }
  void resize(uint32_t new_size, uint32_t new_cap = 0)
  {
    if (new_size == size_) {
      return;
    }
    if (cap_ >= new_size) {
      if (new_size > size_) {
        std::fill(data_ + size_, data_ + new_size, T());
      }
      size_ = new_size;
      return;
    }

    new_cap     = new_size > new_cap ? new_size : new_cap;
    T* new_data = nullptr;
    if (new_cap > 0) {
      new_data = new T[new_cap];
      if (data_ != nullptr) {
        unsigned min_size = std::min(size_, new_size);
        std::move(data_, data_ + min_size, new_data);
      }
    }
    cap_  = new_cap;
    size_ = new_size;
    delete[] data_;
    data_ = new_data;
  }
  iterator erase(iterator it)
  {
    if (it < begin() or it >= end()) {
      log_warning("Trying to access out-of-bounds iterator.");
      return end();
    }

    std::copy(it + 1, end(), it);
    size_--;

    return it;
  }
  bool operator==(const dyn_array<T>& other) const
  {
    return size() == other.size() and std::equal(data_, data_ + size(), other.data_);
  }
  void push_back(const T& elem)
  {
    resize(size() + 1, size() * 2);
    data_[size() - 1] = elem;
  }
  void           clear() { resize(0); }
  T&             back() { return (*this)[size() - 1]; }
  const T&       back() const { return (*this)[size() - 1]; }
  T*             data() { return data_; }
  const T*       data() const { return data_; }
  iterator       begin() { return data_; }
  iterator       end() { return data_ + size(); }
  const_iterator begin() const { return data_; }
  const_iterator end() const { return data_ + size(); }

private:
  T*       data_ = nullptr;
  uint32_t size_ = 0;
  uint32_t cap_  = 0;
};

template <class T, uint32_t MAX_N>
class bounded_array
{
public:
  using value_type     = T;
  using iterator       = T*;
  using const_iterator = const T*;

  bounded_array() = default;
  explicit bounded_array(uint32_t size_) : current_size(size_) {}
  bounded_array(const bounded_array& other) noexcept : current_size(other.size())
  {
    std::copy(other.data(), other.data() + other.size(), data());
  }
  bounded_array& operator=(const bounded_array& other) noexcept
  {
    if (this != &other) {
      current_size = other.size();
      std::copy(other.data(), other.data() + other.size(), data());
    }
    return *this;
  }
  static uint32_t capacity() { return MAX_N; }
  uint32_t        size() const { return current_size; }
  T&              operator[](uint32_t idx) { return data_[idx]; }
  const T&        operator[](uint32_t idx) const { return data_[idx]; }
  bool            operator==(const bounded_array<T, MAX_N>& other) const
  {
    return std::equal(data(), data() + size(), other.data(), other.data() + other.size());
  }
  void resize(uint32_t new_size) { current_size = new_size; }
  void clear() { resize(0); }
  void push_back(const T& elem)
  {
    if (current_size >= MAX_N) {
      log_error("Maximum size {} achieved for bounded_array.", MAX_N);
      return;
    }
    data_[current_size++] = elem;
  }
  T&             back() { return data_[current_size - 1]; }
  const T&       back() const { return data_[current_size - 1]; }
  T*             data() { return data_.data(); }
  const T*       data() const { return data_.data(); }
  iterator       begin() { return data_.data(); }
  iterator       end() { return data_.data() + size(); }
  const_iterator begin() const { return data_.data(); }
  const_iterator end() const { return data_.data() + size(); }

private:
  std::array<T, MAX_N> data_{};
  uint32_t             current_size{0};
};

/**
 * This array does small buffer optimization. The array has a small stack (Nthres elements) to store elements. Once
 * the number of elements exceeds this stack, the array allocs on the heap.
 * @tparam T
 * @tparam Nthres number of elements T that can be stored in the stack
 */
template <class T, uint32_t Nthres = ceil_frac((size_t)16, sizeof(T))>
class ext_array
{
public:
  static const uint32_t small_buffer_size = Nthres;
  ext_array() : size_(0), head(&small_buffer.data[0]) {}
  explicit ext_array(uint32_t new_size) : ext_array() { resize(new_size); }
  ext_array(const ext_array<T, Nthres>& other) : ext_array(other.size_)
  {
    std::copy(other.head, other.head + other.size_, head);
  }
  ext_array(ext_array<T, Nthres>&& other) noexcept
  {
    size_ = other.size();
    if (other.is_in_small_buffer()) {
      head = &small_buffer.data[0];
      std::copy(other.data(), other.data() + other.size(), head);
    } else {
      head              = other.head;
      small_buffer.cap_ = other.small_buffer.cap_;
      other.head        = &other.small_buffer.data[0];
      other.size_       = 0;
    }
  }
  ~ext_array()
  {
    if (not is_in_small_buffer()) {
      delete[] head;
    }
  }
  ext_array<T, Nthres>& operator=(const ext_array<T, Nthres>& other)
  {
    if (this != &other) {
      resize(other.size());
      std::copy(other.data(), other.data() + other.size(), head);
    }
    return *this;
  }

  uint32_t size() const { return size_; }
  uint32_t capacity() const { return is_in_small_buffer() ? Nthres : small_buffer.cap_; }
  T&       operator[](uint32_t index) { return head[index]; }
  const T& operator[](uint32_t index) const { return head[index]; }
  T*       data() { return &head[0]; }
  const T* data() const { return &head[0]; }
  T&       back() { return head[size() - 1]; }
  const T& back() const { return head[size() - 1]; }
  bool     operator==(const ext_array<T, Nthres>& other) const
  {
    return other.size() == size() and std::equal(other.data(), other.data() + other.size(), data());
  }
  void push_back(const T& elem)
  {
    resize(size() + 1);
    head[size() - 1] = elem;
  }
  void resize(uint32_t new_size)
  {
    if (new_size == size_) {
      return;
    }
    if (capacity() >= new_size) {
      size_ = new_size;
      return;
    }
    T*       old_data = head;
    uint32_t newcap   = new_size + 5;
    head              = new T[newcap];
    std::copy(&small_buffer.data[0], &small_buffer.data[size_], head);
    size_ = new_size;
    if (old_data != &small_buffer.data[0]) {
      delete[] old_data;
    }
    small_buffer.cap_ = newcap;
  }
  bool is_in_small_buffer() const { return head == &small_buffer.data[0]; }

private:
  union {
    T        data[Nthres];
    uint32_t cap_;
  } small_buffer;
  uint32_t size_;
  T*       head;
};

/*********************
     ext packing
*********************/

SRSASN_CODE pack_unsupported_ext_flag(bit_ref& bref, bool ext);
SRSASN_CODE unpack_unsupported_ext_flag(bool& ext, bit_ref& bref);

/************************
    asn1 null packing
************************/

struct asn1_null_t {
  SRSASN_CODE pack(bit_ref& bref) const { return SRSASN_SUCCESS; }
  SRSASN_CODE unpack(cbit_ref& bref) const { return SRSASN_SUCCESS; }
};

/************************
     enum packing
************************/

SRSASN_CODE pack_enum(bit_ref& bref, uint32_t enum_val, uint32_t nbits);
SRSASN_CODE pack_enum(bit_ref& bref, uint32_t enum_val, uint32_t nbits, uint32_t nof_noext);
SRSASN_CODE pack_enum(bit_ref& bref, uint32_t e, uint32_t nof_types, uint32_t nof_exts, bool has_ext);
ValOrError  unpack_enum(uint32_t nof_types, uint32_t nof_exts, bool has_ext, cbit_ref& bref);
template <typename EnumType>
SRSASN_CODE pack_enum(bit_ref& bref, EnumType e)
{
  return pack_enum(bref, e, EnumType::nof_types, EnumType::nof_exts, EnumType::has_ext);
}
template <typename EnumType>
SRSASN_CODE unpack_enum(EnumType& e, cbit_ref& bref)
{
  ValOrError ret = unpack_enum(EnumType::nof_types, EnumType::nof_exts, EnumType::has_ext, bref);
  e              = (typename EnumType::options)ret.val;
  return ret.code;
}

struct EnumPacker {
  template <class EnumType>
  SRSASN_CODE pack(bit_ref& bref, EnumType e)
  {
    return pack_enum(bref, e);
  }
  template <class EnumType>
  SRSASN_CODE unpack(EnumType& e, cbit_ref& bref)
  {
    return unpack_enum(e, bref);
  }
};
template <class EnumType>
bool string_to_enum(EnumType& e, const std::string& s)
{
  for (uint32_t i = 0; i < EnumType::nof_types; ++i) {
    e = (typename EnumType::options)i;
    if (e.to_string() == s) {
      return true;
    }
  }
  return false;
}
template <class EnumType, class NumberType>
bool number_to_enum(EnumType& e, NumberType val)
{
  for (uint32_t i = 0; i < e.nof_types; ++i) {
    e = (typename EnumType::options)i;
    if (e.to_number() == val) {
      return true;
    }
  }
  return false;
}
template <class EnumType>
bool number_string_to_enum(EnumType& e, const std::string& val)
{
  for (uint32_t i = 0; i < e.nof_types; ++i) {
    e = (typename EnumType::options)i;
    if (e.to_number_string() == val) {
      return true;
    }
  }
  return false;
}
template <class EnumType>
void bool_to_enum(EnumType& e, bool s)
{
  srsran_assert(e.nof_types == 1, "Can't convert enum with too many values");

  if (s) {
    e = (typename EnumType::options)0;
  } else {
    e = EnumType::nulltype;
  }
}
template <class EnumType>
bool enum_to_bool(EnumType& e)
{
  srsran_assert(e.nof_types == 1, "Can't convert enum with too many values");

  return !(bool)e;
}

template <class EnumType, bool E = false, uint32_t M = 0>
class enumerated : public EnumType
{
public:
  static const uint32_t nof_types = EnumType::nulltype, nof_exts = M;
  static const bool     has_ext = E;

  enumerated() { EnumType::value = EnumType::nulltype; }
  enumerated(typename EnumType::options enumerated_option_val) { EnumType::value = enumerated_option_val; }
  SRSASN_CODE pack(bit_ref& bref) const { return pack_enum(bref, *this); }
  SRSASN_CODE unpack(cbit_ref& bref) { return unpack_enum(*this, bref); }
  EnumType&   operator=(EnumType v)
  {
    EnumType::value = v;
    return *this;
  }
  operator typename EnumType::options() const { return EnumType::value; }
};

/************************
     PER encoding
************************/

/* X.691 - Section 10.5 - Constrained Whole Number */
template <class IntType>
SRSASN_CODE pack_constrained_whole_number(bit_ref& bref, IntType n, IntType lb, IntType ub, bool aligned);
template <class IntType>
SRSASN_CODE unpack_constrained_whole_number(IntType& n, cbit_ref& bref, IntType lb, IntType ub, bool aligned);

/* X.691 - Section 10.6 - Normally small non-negative whole Number */
template <typename UintType>
SRSASN_CODE pack_norm_small_non_neg_whole_number(bit_ref& bref, UintType n);
template <typename UintType>
SRSASN_CODE unpack_norm_small_non_neg_whole_number(UintType& n, cbit_ref& bref);

/* X.691 - Section 10.8 - Unconstrained Whole Number */
template <typename IntType>
SRSASN_CODE pack_unconstrained_whole_number(bit_ref& bref, IntType n, bool aligned);
template <typename IntType>
SRSASN_CODE unpack_unconstrained_whole_number(IntType& n, cbit_ref& bref, bool aligned);

/************************
   length determinant
************************/

// Pack as whole constrained number
template <typename IntType>
SRSASN_CODE pack_length(bit_ref& bref, IntType n, IntType lb, IntType ub, bool aligned = false);
template <typename IntType>
SRSASN_CODE unpack_length(IntType& n, cbit_ref& bref, IntType lb, IntType ub, bool aligned = false);

// Pack as a small non-negative whole number
SRSASN_CODE pack_length(bit_ref& ref, uint32_t val, bool aligned = false);
SRSASN_CODE unpack_length(uint32_t& val, cbit_ref& ref, bool aligned = false);

/************************
        Integer
************************/

template <typename IntType>
SRSASN_CODE pack_integer(bit_ref& bref,
                         IntType  n,
                         IntType  lb      = std::numeric_limits<IntType>::min(),
                         IntType  ub      = std::numeric_limits<IntType>::max(),
                         bool     has_ext = false,
                         bool     aligned = false);
template <typename IntType>
SRSASN_CODE unpack_integer(IntType&  n,
                           cbit_ref& bref,
                           IntType   lb      = std::numeric_limits<IntType>::min(),
                           IntType   ub      = std::numeric_limits<IntType>::max(),
                           bool      has_ext = false,
                           bool      aligned = false);
// unconstrained case
template <typename IntType>
SRSASN_CODE pack_unconstrained_integer(bit_ref& bref, IntType n, bool has_ext = false, bool aligned = false);
template <typename IntType>
SRSASN_CODE unpack_unconstrained_integer(IntType& n, cbit_ref& bref, bool has_ext = false, bool aligned = false);

template <class IntType>
struct integer_packer {
  integer_packer(IntType lb_, IntType ub_, bool has_ext_ = false, bool aligned_ = false);
  SRSASN_CODE pack(bit_ref& bref, IntType n);
  SRSASN_CODE unpack(IntType& n, cbit_ref& bref);
  IntType     lb;
  IntType     ub;
  bool        has_ext;
  bool        aligned;
};

template <class IntType,
          IntType LB  = std::numeric_limits<IntType>::min(),
          IntType UB  = std::numeric_limits<IntType>::max(),
          bool    Ext = false,
          bool    Al  = false>
class integer
{
public:
  static const IntType ub = UB, lb = LB;
  static const bool    has_ext = Ext, is_aligned = Al;
  IntType              value;
  integer() = default;
  integer(IntType value_) : value(value_) {}
  operator IntType() const { return value; }
  SRSASN_CODE pack(bit_ref& bref) const { return pack_integer(bref, value, lb, ub, has_ext, is_aligned); }
  SRSASN_CODE unpack(cbit_ref& bref) { return unpack_integer(value, bref, lb, ub, has_ext, is_aligned); }
};

/************************
  General Packer/Unpacker
************************/

struct BitPacker {
  BitPacker(uint32_t nof_bits_) : nof_bits(nof_bits_) {}
  template <typename T>
  SRSASN_CODE pack(bit_ref& bref, const T& topack)
  {
    bref.pack(topack, nof_bits);
    return SRSASN_SUCCESS;
  }
  template <typename T>
  SRSASN_CODE unpack(T& tounpack, cbit_ref& bref)
  {
    return bref.unpack(tounpack, nof_bits);
  }
  uint32_t nof_bits;
};

struct Packer {
  template <typename T>
  SRSASN_CODE pack(bit_ref& bref, const T& topack)
  {
    return topack.pack(bref);
  }
  template <typename T>
  SRSASN_CODE unpack(T& tounpack, cbit_ref& bref)
  {
    return tounpack.unpack(bref);
  }
};

/*********************
    OCTET STRING
*********************/

namespace octet_string_helper {

/// Convert octet string to an integer.
uint64_t to_uint(srsran::span<const uint8_t> buf);
uint64_t to_uint(const srsran::byte_buffer& buf);

/// Convert unsigned integer to span of octets.
void to_octet_string(srsran::span<uint8_t> buf, uint64_t number);
void to_octet_string(srsran::byte_buffer& buf, uint64_t number);

/// Convert octets into hex string.
std::string to_hex_string(srsran::span<const uint8_t> buf);
std::string to_hex_string(const srsran::byte_buffer& buf);

/// Convert std::string of hexadecimal numbers to span of bytes.
unsigned hex_string_to_octets(srsran::span<uint8_t> buf, const std::string& str);

/// Append std::string of hexadecimal numbers to byte_buffer.
void append_hex_string(srsran::byte_buffer& buf, const std::string& str);

} // namespace octet_string_helper

/************************
    fixed_octstring
************************/

template <uint32_t N, bool aligned = false>
class fixed_octstring
{
public:
  using iterator       = typename std::array<uint8_t, N>::iterator;
  using const_iterator = typename std::array<uint8_t, N>::const_iterator;

  fixed_octstring() = default;
  explicit fixed_octstring(const std::array<uint8_t, N>& array_) : octets_(array_) {}
  fixed_octstring<N, aligned>& operator=(const std::array<uint8_t, N>& array_)
  {
    octets_ = array_;
    return *this;
  }

  const uint8_t& operator[](uint32_t idx) const { return octets_[idx]; }
  uint8_t&       operator[](uint32_t idx) { return octets_[idx]; }
  bool           operator==(const fixed_octstring<N>& other) const { return octets_ == other.octets_; }
  uint8_t*       data() { return &octets_[0]; }
  const uint8_t* data() const { return &octets_[0]; }

  static uint32_t              size() { return N; }
  std::string                  to_string() const { return octet_string_helper::to_hex_string(octets_); }
  fixed_octstring<N, aligned>& from_string(const std::string& hexstr)
  {
    if (hexstr.size() != 2 * N) {
      log_error("The provided hex string size is not valid ({}!=2*{}).", hexstr.size(), (size_t)N);
    } else {
      octet_string_helper::hex_string_to_octets(octets_, hexstr);
    }
    return *this;
  }
  uint64_t                     to_number() const { return octet_string_helper::to_uint(octets_); }
  fixed_octstring<N, aligned>& from_number(uint64_t val)
  {
    octet_string_helper::to_octet_string(octets_, val);
    return *this;
  }
  const std::array<uint8_t, N> to_bytes() const { return octets_; }

  /// Pack fixed-size octet string as per X.691 Section 16 - Encoding the octetstring type.
  /// \tparam N - number of items
  /// \tparam ext - aligned variant
  /// \param bref
  /// \return
  SRSASN_CODE pack(bit_ref& bref) const
  {
    if (aligned and N > 2) {
      bref.align_bytes_zero();
    }
    for (uint32_t i = 0; i < size(); ++i) {
      bref.pack(octets_[i], 8);
    }
    return SRSASN_SUCCESS;
  }
  SRSASN_CODE unpack(cbit_ref& bref)
  {
    if (aligned and N > 2) {
      bref.align_bytes();
    }
    for (uint32_t i = 0; i < size(); ++i) {
      HANDLE_CODE(bref.unpack(octets_[i], 8));
    }
    return SRSASN_SUCCESS;
  }

  iterator       begin() { return octets_.begin(); }
  const_iterator begin() const { return octets_.begin(); }
  iterator       end() { return octets_.end(); }
  const_iterator end() const { return octets_.end(); }

private:
  std::array<uint8_t, N> octets_;
};

/************************
  constrained_octstring
************************/

template <uint32_t LB, uint32_t UB, bool aligned = false>
class bounded_octstring
{
public:
  const uint8_t& operator[](uint32_t idx) const { return octets_[idx]; }
  uint8_t&       operator[](uint32_t idx) { return octets_[idx]; }
  bool           operator==(const bounded_octstring& other) const { return octets_ == other.octets_; }
  uint8_t*       data() { return &octets_[0]; }
  const uint8_t* data() const { return &octets_[0]; }

  void        resize(uint32_t new_size) { octets_.resize(new_size); }
  uint32_t    size() const { return octets_.size(); }
  std::string to_string() const
  {
    return octet_string_helper::to_hex_string(srsran::span<const uint8_t>{data(), size()});
  }
  bounded_octstring<LB, UB, aligned>& from_string(const std::string& hexstr)
  {
    if (hexstr.size() > 2 * UB) {
      log_error("The provided hex string size is not valid ({}>2*{}).", hexstr.size(), (size_t)UB);
    } else {
      resize(hexstr.size() / 2);
      octet_string_helper::hex_string_to_octets(srsran::span<uint8_t>{octets_.data(), octets_.size()}, hexstr);
    }
    return *this;
  }
  uint64_t to_number() const
  {
    return octet_string_helper::to_uint(srsran::span<const uint8_t>{octets_.data(), size()});
  }

  SRSASN_CODE pack(bit_ref& bref) const
  {
    HANDLE_CODE(pack_length(bref, size(), LB, UB, aligned));
    if (aligned) {
      HANDLE_CODE(bref.align_bytes_zero());
    }
    for (uint32_t i = 0; i < size(); ++i) {
      HANDLE_CODE(bref.pack(octets_[i], 8));
    }
    return SRSASN_SUCCESS;
  }
  SRSASN_CODE unpack(cbit_ref& bref)
  {
    uint32_t len;
    HANDLE_CODE(unpack_length(len, bref, LB, UB, aligned));
    resize(len);
    if (aligned) {
      HANDLE_CODE(bref.align_bytes());
    }
    for (uint32_t i = 0; i < size(); ++i) {
      HANDLE_CODE(bref.unpack(octets_[i], 8));
    }
    return SRSASN_SUCCESS;
  }

private:
  ext_array<uint8_t, max_size({UB, 64U})> octets_;
};

/************************
 unbounded OCTET STRING
************************/

template <bool Al = false>
class unbounded_octstring : public srsran::byte_buffer
{
public:
  static const bool aligned = Al;
  using value_type          = byte_buffer::value_type;
  using iterator            = byte_buffer::iterator;
  using const_iterator      = byte_buffer::const_iterator;

  using srsran::byte_buffer::byte_buffer;
  unbounded_octstring(byte_buffer other) noexcept : srsran::byte_buffer(std::move(other)) {}
  unbounded_octstring(const unbounded_octstring& other) noexcept;
  unbounded_octstring(unbounded_octstring&& other) noexcept : srsran::byte_buffer(std::move(other)) {}

  unbounded_octstring& operator=(byte_buffer other) noexcept
  {
    *static_cast<srsran::byte_buffer*>(this) = std::move(other);
    return *this;
  }
  unbounded_octstring& operator=(const unbounded_octstring& other) noexcept;
  unbounded_octstring& operator=(unbounded_octstring&& other) noexcept;

  size_t size() const { return this->length(); }

  std::string              to_string() const;
  unbounded_octstring<Al>& from_string(const std::string& hexstr);
  unbounded_octstring<Al>& from_bytes(srsran::span<const uint8_t> bytes);

  uint64_t                 to_number() const;
  unbounded_octstring<Al>& from_number(uint64_t val);

  srsran::byte_buffer to_byte_buffer() { return std::move(*static_cast<srsran::byte_buffer*>(this)); }

  SRSASN_CODE pack(bit_ref& bref) const;
  SRSASN_CODE unpack(cbit_ref& bref);
};

using dyn_octstring = unbounded_octstring<false>;

/*********************
      bitstring
*********************/

namespace bitstring_utils {
inline bool get(const uint8_t* ptr, uint32_t idx)
{
  uint32_t byte_idx = idx / 8;
  uint32_t offset   = idx % 8;
  return (ptr[byte_idx] & (1u << offset)) > 0;
}
inline void set(uint8_t* ptr, uint32_t idx, bool value)
{
  uint32_t byte_idx = idx / 8;
  uint32_t offset   = idx % 8;
  if (value) {
    ptr[byte_idx] |= (1u << offset);
  } else {
    ptr[byte_idx] &= ((uint16_t)(1u << 8u) - 1u) - (1u << offset);
  }
}
SRSASN_CODE
pack(bit_ref& bref, const uint8_t* data, uint32_t size, uint32_t lb, uint32_t ub, bool has_ext, bool is_aligned);
SRSASN_CODE
unpack_length_prefix(uint32_t& len, cbit_ref& bref, uint32_t lb, uint32_t ub, bool has_ext, bool is_aligned);
SRSASN_CODE unpack_bitfield(uint8_t* buf, cbit_ref& bref, uint32_t n, uint32_t lb, uint32_t ub, bool is_aligned);

uint64_t    to_number(const uint8_t* ptr, uint32_t nbits);
void        from_number(uint8_t* ptr, uint64_t number, uint32_t nbits);
std::string to_string(const uint8_t* ptr, uint32_t nbits);

} // namespace bitstring_utils

template <uint32_t LB, uint32_t UB, bool ext = false, bool aligned = false>
class bitstring
{
  using this_type = bitstring<LB, UB, ext, aligned>;

public:
  static const uint32_t lb = LB, ub = UB;
  static const bool     has_ext = ext, is_aligned = aligned;

  bitstring() { resize(lb); }
  explicit bitstring(uint32_t siz_) { resize(siz_); }
  explicit bitstring(const std::string& s)
  {
    resize(s.size());
    memset(&octets_[0], 0, nof_octets());
    for (uint32_t i = 0; i < s.size(); ++i)
      set(s.size() - i - 1, s[i] == '1');
  }

  bool     get(uint32_t idx) const { return bitstring_utils::get(data(), idx); }
  void     set(uint32_t idx, bool value) { bitstring_utils::set(data(), idx, value); }
  uint32_t nof_octets() const { return ceil_frac(length(), 8u); }

  const uint8_t* data() const { return &octets_[0]; }
  uint8_t*       data() { return &octets_[0]; }
  uint32_t       length() const { return nof_bits; }
  void           resize(uint32_t new_size)
  {
    nof_bits = new_size;
    octets_.resize(nof_octets());
    memset(data(), 0, nof_octets()); // resize always resets content
  }

  // comparison
  bool operator==(const this_type& other) const
  {
    return length() == other.length() and std::equal(data(), data() + nof_octets(), other.data());
  }
  bool operator==(const char* other_str) const
  {
    return strlen(other_str) == length() and (*this) == this_type{}.from_string(other_str);
  }

  // string conversion
  std::string to_string() const { return bitstring_utils::to_string(data(), length()); }
  this_type&  from_string(const std::string& s)
  {
    if (s.size() < lb or s.size() > ub) {
      log_error("The provided string size={} is not withing the bounds [{}, {}]", s.size(), uint32_t(lb), uint32_t(ub));
    } else {
      resize(s.size());
      for (uint32_t i = 0; i < s.size(); ++i) {
        set(s.size() - i - 1, s[i] == '1');
      }
    }
    return *this;
  }

  // number conversion
  uint64_t   to_number() const { return bitstring_utils::to_number(data(), length()); }
  this_type& from_number(uint64_t val)
  {
    auto nof_bits_ = std::max((uint32_t)ceilf(log2(std::max(val, (uint64_t)1u))), LB);
    if (nof_bits_ > UB) {
      log_error("The provided bitstring value {} does not fit the bounds [{}, {}]", val, uint32_t(lb), uint32_t(ub));
      return *this;
    }
    resize(nof_bits_);
    bitstring_utils::from_number(data(), val, length());
    return *this;
  }

  this_type& from_number(uint64_t val, uint32_t nof_bits_)
  {
    if (nof_bits_ > UB) {
      log_error("The provided bitstring value {} does not fit the bounds [{}, {}]", val, uint32_t(lb), uint32_t(ub));
      return *this;
    }
    resize(nof_bits_);
    bitstring_utils::from_number(data(), val, length());
    return *this;
  }

  // packers / unpackers
  SRSASN_CODE pack(bit_ref& bref) const
  {
    return bitstring_utils::pack(bref, data(), length(), lb, ub, has_ext, is_aligned);
  }
  SRSASN_CODE unpack(cbit_ref& bref)
  {
    // X.691, subclause 15.11
    uint32_t nbits;
    HANDLE_CODE(bitstring_utils::unpack_length_prefix(nbits, bref, lb, ub, has_ext, is_aligned));
    resize(nbits);
    return bitstring_utils::unpack_bitfield(data(), bref, nbits, lb, ub, is_aligned);
  }

private:
  static const uint32_t          stack_size = (UB == std::numeric_limits<uint32_t>::max()) ? 4 : ceil_frac(ub, 8u);
  ext_array<uint8_t, stack_size> octets_;
  uint32_t                       nof_bits = 0;
};

template <uint32_t LB, uint32_t UB, bool ext = false, bool aligned = false>
using bounded_bitstring = bitstring<LB, UB, ext, aligned>;

template <bool Ext = false, bool Al = false>
using unbounded_bitstring = bitstring<0, std::numeric_limits<uint32_t>::max(), Ext, Al>;

using dyn_bitstring = unbounded_bitstring<false, false>;

template <uint32_t N, bool Ext = false, bool Al = false>
using fixed_bitstring = bitstring<N, N, Ext, Al>;

/*********************
  fixed sequence of
*********************/

// packers/unpackers for fixed_length sequence-of
template <class ArrayType, class ItemPacker>
SRSASN_CODE pack_fixed_seq_of(bit_ref& bref, const ArrayType& item_array, uint32_t nof_items, ItemPacker packer)
{
  for (uint32_t i = 0; i < nof_items; ++i) {
    HANDLE_CODE(packer.pack(bref, item_array[i]));
  }
  return SRSASN_SUCCESS;
}
template <class ArrayType>
SRSASN_CODE pack_fixed_seq_of(bit_ref& bref, const ArrayType& item_array, uint32_t nof_items)
{
  for (uint32_t i = 0; i < nof_items; ++i) {
    HANDLE_CODE(item_array[i].pack(bref));
  }
  return SRSASN_SUCCESS;
}
template <class ArrayType, class ItemUnpacker>
SRSASN_CODE unpack_fixed_seq_of(ArrayType& item_array, cbit_ref& bref, uint32_t nof_items, ItemUnpacker unpacker)
{
  for (uint32_t i = 0; i < nof_items; ++i) {
    HANDLE_CODE(unpacker.unpack(item_array[i], bref));
  }
  return SRSASN_SUCCESS;
}
template <class ArrayType>
SRSASN_CODE unpack_fixed_seq_of(ArrayType& item_array, cbit_ref& bref, uint32_t nof_items)
{
  for (uint32_t i = 0; i < nof_items; ++i) {
    HANDLE_CODE(item_array[i].unpack(bref));
  }
  return SRSASN_SUCCESS;
}

template <class ItemPacker>
struct FixedSeqOfPacker {
  FixedSeqOfPacker(uint32_t nof_items_, ItemPacker packer_) : nof_items(nof_items_), packer(packer_) {}
  explicit FixedSeqOfPacker(uint32_t nof_items_) : nof_items(nof_items_), packer(Packer()) {}
  template <typename T>
  SRSASN_CODE pack(bit_ref& bref, const T& topack)
  {
    return pack_fixed_seq_of(bref, topack, nof_items, packer);
  }
  template <typename T>
  SRSASN_CODE unpack(T& tounpack, cbit_ref& bref)
  {
    return unpack_fixed_seq_of(tounpack, bref, nof_items, packer);
  }
  uint32_t   nof_items;
  ItemPacker packer;
};

/*********************
   dyn sequence of
*********************/

template <class ArrayType, class ItemPacker>
SRSASN_CODE pack_dyn_seq_of(bit_ref&         bref,
                            const ArrayType& seqof,
                            uint32_t         lb,
                            uint32_t         ub,
                            ItemPacker       packer,
                            bool             aligned = false)
{
  HANDLE_CODE(pack_length(bref, seqof.size(), lb, ub, aligned));
  for (uint32_t i = 0; i < seqof.size(); ++i) {
    HANDLE_CODE(packer.pack(bref, seqof[i]));
  }
  return SRSASN_SUCCESS;
}

template <class ArrayType>
SRSASN_CODE pack_dyn_seq_of(bit_ref& bref, const ArrayType& seqof, uint32_t lb, uint32_t ub, bool aligned = false)
{
  HANDLE_CODE(pack_length(bref, seqof.size(), lb, ub, aligned));
  for (uint32_t i = 0; i < seqof.size(); ++i) {
    HANDLE_CODE(seqof[i].pack(bref));
  }
  return SRSASN_SUCCESS;
}

template <class ArrayType, class ItemUnpacker>
SRSASN_CODE unpack_dyn_seq_of(ArrayType&   seqof,
                              cbit_ref&    bref,
                              uint32_t     lb,
                              uint32_t     ub,
                              ItemUnpacker unpacker,
                              bool         aligned = false)
{
  uint32_t nof_items;
  HANDLE_CODE(unpack_length(nof_items, bref, lb, ub, aligned));
  seqof.resize(nof_items);
  for (uint32_t i = 0; i < nof_items; ++i) {
    HANDLE_CODE(unpacker.unpack(seqof[i], bref));
  }
  return SRSASN_SUCCESS;
}

template <class ArrayType>
SRSASN_CODE unpack_dyn_seq_of(ArrayType& seqof, cbit_ref& bref, uint32_t lb, uint32_t ub, bool aligned = false)
{
  uint32_t nof_items;
  HANDLE_CODE(unpack_length(nof_items, bref, lb, ub, aligned));
  seqof.resize(nof_items);
  for (uint32_t i = 0; i < nof_items; ++i) {
    HANDLE_CODE(seqof[i].unpack(bref));
  }
  return SRSASN_SUCCESS;
}

template <class InnerPacker>
struct SeqOfPacker {
  SeqOfPacker(uint32_t lb_, uint32_t ub_, InnerPacker packer_) : lb(lb_), ub(ub_), packer(packer_) {}
  template <typename T>
  SRSASN_CODE pack(bit_ref& bref, const T& topack) const
  {
    return pack_dyn_seq_of(bref, topack, lb, ub, packer);
  }
  template <typename T>
  SRSASN_CODE unpack(T& tounpack, cbit_ref& bref)
  {
    return unpack_dyn_seq_of(tounpack, bref, lb, ub, packer);
  }
  uint32_t    lb;
  uint32_t    ub;
  InnerPacker packer;
};

template <class ItemType, uint32_t lb, uint32_t ub, bool aligned = false>
struct dyn_seq_of : public dyn_array<ItemType> {
  dyn_seq_of() = default;
  dyn_seq_of(const dyn_array<ItemType>& other) : dyn_array<ItemType>(other) {}
  dyn_seq_of(const bounded_array<ItemType, ub>& other) : dyn_array<ItemType>(&other[0], other.size()) {}
  SRSASN_CODE pack(bit_ref& bref) const { return pack_dyn_seq_of(bref, *this, lb, ub, aligned); }
  SRSASN_CODE unpack(cbit_ref& bref) { return unpack_dyn_seq_of(*this, bref, lb, ub, aligned); }
};

/*********************
   printable string
*********************/

/* X.691 - Section 27 - Character Restricted String */
namespace asn_string_utils {
SRSASN_CODE
pack(bit_ref& bref, const std::string& s, size_t lb, size_t ub, size_t alb, size_t aub, bool ext, bool aligned);
SRSASN_CODE
unpack(std::string& s, cbit_ref& bref, size_t lb, size_t ub, size_t alb, size_t aub, bool ext, bool aligned);
} // namespace asn_string_utils

constexpr uint32_t MAX_ASN_STRING_LENGTH = std::numeric_limits<uint32_t>::max();

template <uint32_t LB,
          uint32_t UB,
          uint32_t ALB     = 0,
          uint32_t AUB     = MAX_ASN_STRING_LENGTH,
          bool     ext     = false,
          bool     aligned = false>
class asn_string
{
public:
  SRSASN_CODE pack(bit_ref& bref) const { return asn_string_utils::pack(bref, str, LB, UB, ALB, AUB, ext, aligned); }
  SRSASN_CODE unpack(cbit_ref& bref) { return asn_string_utils::unpack(str, bref, LB, UB, ALB, AUB, ext, aligned); }
  char&       operator[](std::size_t idx) { return str[idx]; }
  const char& operator[](std::size_t idx) const { return str[idx]; }
  void        resize(std::size_t newsize) { str.resize(newsize); }
  std::size_t size() const { return str.size(); }
  std::string to_string() const { return str; }
  void        from_string(const std::string& s) { str = s; }

private:
  std::string str;
};

template <uint32_t ALB = 0, uint32_t AUB = MAX_ASN_STRING_LENGTH, bool ext = false, bool aligned = false>
using printable_string = asn_string<32, 122, ALB, AUB, ext, aligned>;

// TODO: Support UTF8String.
template <uint32_t ALB = 0, uint32_t AUB = MAX_ASN_STRING_LENGTH, bool ext = false, bool aligned = false>
using utf8_string = asn_string<0, 255, ALB, AUB, ext, aligned>;

template <uint32_t ALB = 0, uint32_t AUB = MAX_ASN_STRING_LENGTH, bool ext = false, bool aligned = false>
using visible_string = asn_string<32, 126, ALB, AUB, ext, aligned>;

/*********************
      copy_ptr
*********************/

template <class T>
class copy_ptr
{
public:
  copy_ptr() : ptr(nullptr) {}
  explicit copy_ptr(T* ptr_) : ptr(ptr_) {}
  copy_ptr(copy_ptr<T>&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }
  copy_ptr(const copy_ptr<T>& other) { ptr = (other.ptr == nullptr) ? nullptr : new T(*other.ptr); }
  ~copy_ptr() { destroy_(); }
  copy_ptr<T>& operator=(const copy_ptr<T>& other)
  {
    if (this != &other) {
      reset((other.ptr == nullptr) ? nullptr : new T(*other.ptr));
    }
    return *this;
  }
  copy_ptr<T>& operator=(copy_ptr<T>&& other) noexcept
  {
    if (this != &other) {
      ptr       = other.ptr;
      other.ptr = nullptr;
    }
    return *this;
  }
  bool     operator==(const copy_ptr<T>& other) const { return *ptr == *other; }
  T*       operator->() { return ptr; }
  const T* operator->() const { return ptr; }
  T&       operator*() { return *ptr; }       // like pointers, don't call this if ptr==NULL
  const T& operator*() const { return *ptr; } // like pointers, don't call this if ptr==NULL
  T*       get() { return ptr; }
  const T* get() const { return ptr; }
  T*       release()
  {
    T* ret = ptr;
    ptr    = nullptr;
    return ret;
  }
  void reset(T* ptr_ = nullptr)
  {
    destroy_();
    ptr = ptr_;
  }
  void set_present(bool flag = true)
  {
    if (flag) {
      reset(new T());
    } else {
      reset();
    }
  }
  bool is_present() const { return get() != nullptr; }

private:
  void destroy_()
  {
    if (ptr != NULL) {
      delete ptr;
    }
  }
  T* ptr;
};

template <class T>
copy_ptr<typename std::decay<T>::type> make_copy_ptr(T&& t)
{
  using T2 = typename std::decay<T>::type;
  return copy_ptr<T2>(new T2(std::forward<T>(t)));
}

/// Unpack a boolean value indicating whether an optional field is present.
template <class T>
SRSASN_CODE unpack_presence_flag(copy_ptr<T>& optional_field, cbit_ref& bref)
{
  bool        present_flag;
  SRSASN_CODE ret = bref.unpack(present_flag, 1);
  if (ret != SRSASN_SUCCESS) {
    return ret;
  }
  optional_field.set_present(present_flag);
  return ret;
}

/*********************
     choice utils
*********************/

union alignment_t {
  char        c;
  float       f;
  uint32_t    i;
  uint64_t    i2;
  double      d;
  long double d2;
  uint32_t*   ptr;
};

template <std::size_t Size, std::size_t Align>
struct choice_buffer_base_t {
  static const std::size_t data_size  = Size;
  static const std::size_t data_align = Align;
  using buffer_t                      = typename std::aligned_storage<data_size, data_align>::type;
  buffer_t buffer;

  choice_buffer_base_t() : buffer() {}

  template <typename T>
  T& get()
  {
    return *std::launder(reinterpret_cast<T*>(&buffer));
  }
  template <typename T>
  const T& get() const
  {
    return *std::launder(reinterpret_cast<const T*>(&buffer));
  }
  template <typename T>
  void destroy()
  {
    get<T>().~T();
  }
  template <typename T>
  void init()
  {
    new (&buffer) T();
  }
  template <typename T>
  void init(const T& other)
  {
    new (&buffer) T(other);
  }
  template <typename T>
  void set(const T& other)
  {
    get<T>() = other;
  }
};

template <typename... Ts>
struct choice_buffer_t : public choice_buffer_base_t<max_size({sizeof(alignment_t), sizeof(Ts)...}),
                                                     max_size({alignof(alignment_t), alignof(Ts)...})> {};

using pod_choice_buffer_t = choice_buffer_t<>;

struct choice_buffer_ptr {
public:
  choice_buffer_ptr() = default;
  template <typename T>
  choice_buffer_ptr(T&& to_store) : obj(std::forward<T>(to_store))
  {
  }

  template <typename T, std::enable_if_t<not std::is_same<std::decay_t<T>, choice_buffer_ptr>::value, int> = 0>
  choice_buffer_ptr& operator=(T&& to_store)
  {
    obj = std::forward<T>(to_store);
    return *this;
  }

  bool has_value() const { return obj.has_value(); }

  template <typename T>
  bool holds_choice() const
  {
    return std::any_cast<T>(&obj) != nullptr;
  }

  template <typename T>
  T& get()
  {
    T* ret = std::any_cast<T>(&obj);
    srsran_assert(ret != nullptr, "Invalid choice type");
    return *ret;
  }

  template <typename T>
  const T& get() const
  {
    const T* ret = std::any_cast<T>(&obj);
    srsran_assert(ret != nullptr, "Invalid choice type");
    return *ret;
  }

  void reset() { obj.reset(); }

private:
  std::any obj;
};

/*********************
      ext group
*********************/

class ext_groups_packer_guard
{
public:
  bool&       operator[](uint32_t idx);
  SRSASN_CODE pack(bit_ref& bref) const;

private:
  ext_array<bool> groups;
};

class ext_groups_unpacker_guard
{
public:
  explicit ext_groups_unpacker_guard(uint32_t nof_supported_groups_);
  ~ext_groups_unpacker_guard();

  void        resize(uint32_t new_size);
  bool&       operator[](uint32_t idx);
  SRSASN_CODE unpack(cbit_ref& bref);

private:
  ext_array<bool> groups;
  const uint32_t  nof_supported_groups;
  uint32_t        nof_unpacked_groups = 0;
  cbit_ref*       bref_tracker        = nullptr;
};

/*********************
   Var Length Field
*********************/

class varlength_field_pack_guard
{
public:
  explicit varlength_field_pack_guard(bit_ref& bref, bool align_ = false);
  ~varlength_field_pack_guard();

private:
  bit_ref             brefstart;
  bit_ref*            bref_tracker;
  srsran::byte_buffer varlen_buffer;
  bool                align;
};

class varlength_field_unpack_guard
{
public:
  explicit varlength_field_unpack_guard(cbit_ref& bref, bool align = false);
  ~varlength_field_unpack_guard();

private:
  uint32_t  len = 0;
  cbit_ref  bref0;
  cbit_ref* bref_tracker = nullptr;
};

/*******************
    JsonWriter
*******************/

using json_buffer = fmt::basic_memory_buffer<char, 2048>;

class json_writer
{
public:
  json_writer();
  void        write_fieldname(const char* fieldname);
  void        write_str(const char* fieldname, const std::string& value);
  void        write_str(const char* fieldname, const char* value);
  void        write_str(const char* value);
  void        write_str(const std::string& value);
  void        write_int(const char* fieldname, int64_t value);
  void        write_int(int64_t value);
  void        write_bool(const char* fieldname, bool value);
  void        write_bool(bool value);
  void        write_float(const char* fieldname, float value);
  void        write_float(float value);
  void        write_null(const char* fieldname);
  void        write_null();
  void        start_obj(const char* fieldname = "");
  void        end_obj();
  void        start_array(const char* fieldname = "");
  void        end_array();
  std::string to_string() const;

private:
  json_buffer buffer;
  std::string ident;
  enum separator_t { COMMA = 0, NEWLINE, NONE };
  separator_t sep;
};

template <typename T>
inline auto to_json(json_writer& j, const T& obj) -> decltype(obj.to_json(j))
{
  obj.to_json(j);
}

template <typename T>
inline auto to_json(json_writer& j, const T& obj) -> decltype(j.write_str(obj.to_string()))
{
  j.write_str(obj.to_string());
}

template <typename T>
inline void to_json(json_writer& j, const asn1::dyn_array<T>& lst)
{
  j.start_array();
  for (const auto& o : lst) {
    to_json(j, o);
  }
  j.end_array();
}

template <typename T, size_t N>
inline void to_json(json_writer& j, const std::array<T, N>& lst)
{
  j.start_array();
  for (const auto& o : lst) {
    to_json(j, o);
  }
  j.end_array();
}

inline void to_json(json_writer& j, int64_t number)
{
  j.write_int(number);
}

/****************
 REAL type
***************/

SRSASN_CODE pack_unconstrained_real(bit_ref& bref, float n, bool aligned = false);

SRSASN_CODE unpack_unconstrained_real(float& n, cbit_ref& bref, bool aligned = false);

class real_s
{
public:
  float value;
  real_s() = default;
  SRSASN_CODE pack(bit_ref& bref) const { return pack_unconstrained_real(bref, value, true); }
  SRSASN_CODE unpack(cbit_ref& bref) { return unpack_unconstrained_real(value, bref, true); }
  void        to_json(json_writer& j) const { j.write_float(value); }
};

/*******************
  Test pack/unpack
*******************/

template <class Msg>
int test_pack_unpack_consistency(const Msg& msg)
{
  srsran::byte_buffer buf, buf2;
  Msg                 msg2;
  asn1::bit_ref       bref(buf), bref3(buf2);

  if (msg.pack(bref) != asn1::SRSASN_SUCCESS) {
    log_error_code(SRSASN_ERROR_ENCODE_FAIL, __FILE__, __LINE__);
    return -1;
  }

  asn1::cbit_ref bref2(bref);
  if (msg2.unpack(bref2) != asn1::SRSASN_SUCCESS) {
    log_error_code(SRSASN_ERROR_DECODE_FAIL, __FILE__, __LINE__);
    return -1;
  }
  if (msg2.pack(bref3) != asn1::SRSASN_SUCCESS) {
    log_error_code(SRSASN_ERROR_ENCODE_FAIL, __FILE__, __LINE__);
    return -1;
  }

  // unpack and last pack done for the same number of bits
  if (bref3.distance() != bref2.distance()) {
    log_error("[{}][{}] .", __FILE__, __LINE__);
    return -1;
  }

  // ensure packed messages are the same
  if (bref3.distance() != bref.distance()) {
    log_error("[{}][{}] .", __FILE__, __LINE__);
    return -1;
  }
  if (buf != buf2) {
    log_error("[{}][{}] .", __FILE__, __LINE__);
    return -1;
  }
  return SRSASN_SUCCESS;
}

/************************
   General Layer Types
************************/

/// Enumerated used in RRC and RRC NR that distinguishes Release and Setup modes
struct setup_release_opts {
  enum options { release, setup, nulltype } value;

  const char* to_string() const
  {
    static const char* names[] = {"release", "setup"};
    return convert_enum_idx(names, 2, value, "setup_release_c::types");
  }
};
using setup_release_e = enumerated<setup_release_opts>;

// SetupRelease{ElementTypeParam} ::= CHOICE
template <class T>
struct setup_release_c {
  using types_opts = setup_release_opts;
  using types      = setup_release_e;

  // choice methods
  setup_release_c() = default;
  void        set(typename types::options e = types::nulltype) { type_ = e; }
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const
  {
    type_.pack(bref);
    switch (type_) {
      case types::release:
        break;
      case types::setup:
        HANDLE_CODE(c.pack(bref));
        break;
      default:
        log_invalid_choice_id(type_, "setup_release_c");
        return SRSASN_ERROR_ENCODE_FAIL;
    }
    return SRSASN_SUCCESS;
  }
  SRSASN_CODE unpack(cbit_ref& bref)
  {
    types e;
    e.unpack(bref);
    set(e);
    switch (type_) {
      case types::release:
        break;
      case types::setup:
        HANDLE_CODE(c.unpack(bref));
        break;
      default:
        log_invalid_choice_id(type_, "setup_release_c");
        return SRSASN_ERROR_DECODE_FAIL;
    }
    return SRSASN_SUCCESS;
  }
  void to_json(json_writer& j) const
  {
    j.start_obj();
    switch (type_) {
      case types::release:
        break;
      case types::setup:
        asn1::to_json(j, setup());
        break;
      default:
        log_invalid_choice_id(type_, "setup_release_c");
    }
    j.end_obj();
  }
  // getters
  bool is_setup() const { return type_.value == setup_release_opts::setup; }
  T&   setup()
  {
    assert_choice_type(types::setup, type_, "SetupRelease");
    return c;
  }
  const T& setup() const
  {
    assert_choice_type(types::setup, type_, "SetupRelease");
    return c;
  }
  void set_release() { set(types::release); }
  T&   set_setup()
  {
    set(types::setup);
    return c;
  }

  bool operator==(const setup_release_c<T>& other) const
  {
    return type_ == other.type_ and (type_ != types::setup or (c == other.c));
  }

private:
  types type_;
  T     c;
};

template <typename T, size_t N>
struct setup_release_c<std::array<T, N>> {
  using types_opts = setup_release_opts;
  using types      = setup_release_e;

  // choice methods
  setup_release_c() = default;
  void        set(typename types::options e = types::nulltype) { type_ = e; }
  types       type() const { return type_; }
  SRSASN_CODE pack(bit_ref& bref) const
  {
    type_.pack(bref);
    switch (type_) {
      case types::release:
        break;
      case types::setup:
        HANDLE_CODE(pack_fixed_seq_of(bref, c, N));
        break;
      default:
        log_invalid_choice_id(type_, "setup_release_c");
        return SRSASN_ERROR_ENCODE_FAIL;
    }
    return SRSASN_SUCCESS;
  }
  SRSASN_CODE unpack(cbit_ref& bref)
  {
    types e;
    e.unpack(bref);
    set(e);
    switch (type_) {
      case types::release:
        break;
      case types::setup:
        HANDLE_CODE(unpack_fixed_seq_of(c, bref, N));
        break;
      default:
        log_invalid_choice_id(type_, "setup_release_c");
        return SRSASN_ERROR_DECODE_FAIL;
    }
    return SRSASN_SUCCESS;
  }
  void to_json(json_writer& j) const
  {
    j.start_obj();
    switch (type_) {
      case types::release:
        break;
      case types::setup:
        asn1::to_json(j, setup());
        break;
      default:
        log_invalid_choice_id(type_, "setup_release_c");
    }
    j.end_obj();
  }
  // getters
  bool              is_setup() const { return type_.value == setup_release_opts::setup; }
  std::array<T, N>& setup()
  {
    assert_choice_type(types::setup, type_, "SetupRelease");
    return c;
  }
  const std::array<T, N>& setup() const
  {
    assert_choice_type(types::setup, type_, "SetupRelease");
    return c;
  }
  void              set_release() { set(types::release); }
  std::array<T, N>& set_setup()
  {
    set(types::setup);
    return c;
  }

private:
  types            type_;
  std::array<T, N> c;
};

// Criticality ::= ENUMERATED
struct crit_opts {
  enum options { reject, ignore, notify, nulltype } value;
  const char* to_string() const
  {
    static const char* names[] = {"reject", "ignore", "notify"};
    return convert_enum_idx(names, 3, value, "crit_e");
  }
};
typedef enumerated<crit_opts> crit_e;

// Presence ::= ENUMERATED
struct presence_opts {
  enum options { optional, conditional, mandatory, nulltype } value;

  const char* to_string() const
  {
    static const char* names[] = {"optional", "conditional", "mandatory"};
    return convert_enum_idx(names, 3, value, "presence_e");
  }
};
typedef enumerated<presence_opts> presence_e;

} // namespace asn1
