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

#include "srsran/adt/byte_buffer_view.h"
#include "srsran/adt/detail/byte_buffer_memory_resource.h"
#include "srsran/adt/detail/intrusive_ptr.h"
#include "srsran/adt/expected.h"
#include "fmt/format.h"

namespace srsran {

/// Default byte buffer segment size in the byte buffer pool.
size_t byte_buffer_segment_pool_default_segment_size();

/// \brief Sets the default size of the default byte_buffer segment pool.
///
/// This function asserts if the pool has already been initialized.
/// \param nof_segments Number of byte_buffer_segments stored in the pool.
/// \param memory_block_size Number, in bytes, for each memory block on which a buffer_byte_segment header and payload
/// will be stored.
void init_byte_buffer_segment_pool(std::size_t nof_segments,
                                   std::size_t memory_block_size = byte_buffer_segment_pool_default_segment_size());

/// \brief Get total capacity of the byte buffer segment pool in terms of segments.
size_t get_byte_buffer_segment_pool_capacity();

/// \brief Get an estimate of the number of segments of the byte buffer segment pool that are currently available for
/// allocation in the caller thread.
size_t get_byte_buffer_segment_pool_current_size_approx();

/// \brief Default byte buffer segment pool.
byte_buffer_memory_resource& get_default_byte_buffer_segment_pool();

/// \brief Default byte buffer segment pool with fallback to the heap on failure to allocate.
byte_buffer_memory_resource& get_default_fallback_byte_buffer_segment_pool();

/// \brief Preinitialize the byte buffer segment pool thread-local storage.
/// This method is useful to avoid the overhead of thread_local initialization in latency-critical parts of the code.
void init_byte_buffer_segment_pool_tls();

class byte_buffer_slice;

/// \brief Byte sequence, which represents its data in memory via an intrusive linked list of memory chunks.
///
/// This container is not contiguous in memory.
/// Default copy ctor, assignment and explicit construction is disabled in this container. The user should use the
/// provided factory methods to create objects, use std::move to transfer ownership, .copy() for shallow copies with
/// shared ownership and .deep_copy() for byte-wise copies.
class byte_buffer
{
  /// Node of linked list of byte buffer segments.
  using node_t = detail::byte_buffer_segment_list::node_t;

  /// Control block of byte_buffer used to store the head and tail of the segment linked list and the number of total
  /// bytes of the buffer.
  struct control_block {
    /// Linked list of byte buffer segments that this byte_buffer holds.
    detail::byte_buffer_segment_list segments;
    /// Length in bytes of the linked list of segments.
    size_t pkt_len = 0;
    /// One of the segments shares the same memory block with the byte_buffer control block.
    node_t* segment_in_cb_memory_block = nullptr;
    /// Intrusive ptr reference counter.
    intrusive_ptr_atomic_ref_counter ref_count;
    /// Memory resource used to allocate/deallocate segments.
    byte_buffer_memory_resource* segment_pool;

    void destroy_node(node_t* node) const;

    ~control_block();

  private:
    friend void intrusive_ptr_inc_ref(control_block* ptr) { ptr->ref_count.inc_ref(); }
    friend void intrusive_ptr_dec_ref(control_block* ptr)
    {
      if (ptr->ref_count.dec_ref()) {
        ptr->destroy_cb();
      }
    }

    void destroy_cb();
  };

  /// Headroom given to the first segment of the byte_buffer.
  static constexpr size_t DEFAULT_FIRST_SEGMENT_HEADROOM = 16;

public:
  using value_type     = uint8_t;
  using iterator       = detail::byte_buffer_segment_list_byte_iterator;
  using const_iterator = detail::byte_buffer_segment_list_byte_const_iterator;

  struct fallback_allocation_tag {};

  /// Creates an empty byte_buffer.
  byte_buffer() noexcept = default;

  /// Explicit copy ctor. User should use copy() method for copy assignments.
  explicit byte_buffer(const byte_buffer&) noexcept = default;

  /// Move constructor.
  byte_buffer(byte_buffer&& other) noexcept = default;

  /// Creates an empty byte_buffer with a custom segment memory pool.
  static expected<byte_buffer> create(byte_buffer_memory_resource& segment_pool);

  /// Creates a byte_buffer with contents provided by a span of bytes.
  /// \param[in] bytes span of bytes to assign to the byte_buffer.
  /// \param[in] segment_pool memory pool used to allocate segments.
  static expected<byte_buffer>
  create(span<const uint8_t> bytes, byte_buffer_memory_resource& segment_pool = get_default_byte_buffer_segment_pool());

  /// Creates a byte_buffer with data initialized via an initializer list.
  /// \param[in] lst initializer list with bytes to assign to the byte_buffer.
  /// \param[in] pool memory pool used to allocate segments.
  static expected<byte_buffer> create(const std::initializer_list<uint8_t>& lst,
                                      byte_buffer_memory_resource& pool = get_default_byte_buffer_segment_pool());

  /// Creates a byte_buffer with data assigned from a range of bytes.
  template <typename It>
  static expected<byte_buffer> create(It other_begin, It other_end)
  {
    byte_buffer buf;
    if (not buf.append(other_begin, other_end)) {
      return make_unexpected(default_error_t{});
    }
    return buf;
  }

  /// Creates a byte_buffer that in case it fails to allocate from the default pool, it resorts to malloc as fallback.
  byte_buffer(fallback_allocation_tag tag, span<const uint8_t> other = {}) noexcept;
  byte_buffer(fallback_allocation_tag tag, const std::initializer_list<uint8_t>& other) noexcept;
  byte_buffer(fallback_allocation_tag tag, const byte_buffer& other) noexcept;

  /// Copy assignment operator is disabled. Use std::move, .copy() or .deep_copy() instead.
  byte_buffer& operator=(const byte_buffer&) noexcept = delete;

  /// Move assignment operator of byte_buffer. It avoids unnecessary reference counting increment.
  byte_buffer& operator=(byte_buffer&& other) noexcept = default;

  /// Performs a deep copy (byte by bytes) of this byte_buffer.
  expected<byte_buffer> deep_copy() const;
  expected<byte_buffer> deep_copy(fallback_allocation_tag tag) const;

  /// Performs a shallow copy. Head segment reference counter is incremented.
  byte_buffer copy() const { return byte_buffer{*this}; }

  /// Append bytes of a iterator range.
  template <typename Iterator>
  [[nodiscard]] bool append(Iterator begin, Iterator end)
  {
    static_assert(std::is_same_v<std::decay_t<decltype(*begin)>, uint8_t> or
                      std::is_same_v<std::decay_t<decltype(*begin)>, const uint8_t>,
                  "Iterator value type is not uint8_t");
    using iter_category = typename std::iterator_traits<Iterator>::iterator_category;

    if constexpr (std::is_same_v<iter_category, std::random_access_iterator_tag>) {
      return append(span<const uint8_t>(&*begin, &*end));
    }
    // TODO: use segment-wise copy if it is a byte buffer-like type.
    for (auto it = begin; it != end; ++it) {
      if (not append(*it)) {
        return false;
      }
    }
    return true;
  }

  /// Appends bytes to the byte buffer. This function may retrieve new segments from a memory pool.
  [[nodiscard]] bool append(span<const uint8_t> bytes);

  /// Appends an initializer list of bytes.
  [[nodiscard]] bool append(const std::initializer_list<uint8_t>& bytes);

  /// Appends bytes from another byte_buffer. This function may allocate new segments.
  [[nodiscard]] bool append(const byte_buffer& other);

  /// Appends bytes from another rvalue byte_buffer. This function may allocate new segments.
  [[nodiscard]] bool append(byte_buffer&& other);

  /// Appends bytes to the byte buffer. This function may allocate new segments.
  [[nodiscard]] bool append(uint8_t byte)
  {
    if (empty() or ctrl_blk_ptr->segments.tail->tailroom() == 0) {
      if (not append_segment(DEFAULT_FIRST_SEGMENT_HEADROOM, byte_buffer_segment_pool_default_segment_size())) {
        return false;
      }
    }
    ctrl_blk_ptr->segments.tail->append(byte);
    ctrl_blk_ptr->pkt_len++;
    return true;
  }

  /// Appends a view of bytes into current byte buffer.
  [[nodiscard]] bool append(const byte_buffer_view& view);

  /// Appends an owning view of bytes into current byte buffer.
  [[nodiscard]] bool append(const byte_buffer_slice& slice);

  /// Prepends bytes to byte_buffer. This function may allocate new segments.
  [[nodiscard]] bool prepend(span<const uint8_t> bytes);

  /// \brief Prepend data of byte buffer to this byte buffer.
  [[nodiscard]] bool prepend(const byte_buffer& other);

  /// \brief Prepend data of r-value byte buffer to this byte buffer. The segments of the provided byte buffer can get
  /// "stolen" if the byte buffer is the last reference to the segments.
  [[nodiscard]] bool prepend(byte_buffer&& other);

  /// Prepends space in byte_buffer. This function may allocate new segments.
  /// \param nof_bytes Number of bytes to reserve at header.
  /// \return range of bytes that were reserved.
  byte_buffer_view reserve_prepend(size_t nof_bytes);

  /// Clear byte buffer.
  void clear() { ctrl_blk_ptr.reset(); }

  /// Removes "nof_bytes" from the head of the byte_buffer.
  void trim_head(size_t nof_bytes);

  /// \brief Remove "nof_bytes" bytes at the end of the byte_buffer.
  /// If the length is greater than the length of the last segment, the function will fail and return -1 without
  /// modifying the byte_buffer.
  void trim_tail(size_t nof_bytes);

  /// Checks whether byte_buffer is empty.
  bool empty() const { return length() == 0; }

  /// Checks byte_buffer length.
  size_t length() const { return ctrl_blk_ptr != nullptr ? ctrl_blk_ptr->pkt_len : 0; }

  uint8_t&       back() { return ctrl_blk_ptr->segments.tail->back(); }
  const uint8_t& back() const { return ctrl_blk_ptr->segments.tail->back(); }

  const uint8_t& operator[](size_t i) const { return *(begin() + i); }
  uint8_t&       operator[](size_t i) { return *(begin() + i); }

  iterator       begin() { return iterator{ctrl_blk_ptr != nullptr ? ctrl_blk_ptr->segments.head : nullptr, 0}; }
  const_iterator cbegin() const
  {
    return const_iterator{ctrl_blk_ptr != nullptr ? ctrl_blk_ptr->segments.head : nullptr, 0};
  }
  const_iterator begin() const
  {
    return const_iterator{ctrl_blk_ptr != nullptr ? ctrl_blk_ptr->segments.head : nullptr, 0};
  }
  iterator       end() { return iterator{nullptr, 0}; }
  const_iterator end() const { return const_iterator{nullptr, 0}; }
  const_iterator cend() const { return const_iterator{nullptr, 0}; }

  /// Test if byte buffer is contiguous in memory, i.e. it has only one segment.
  bool is_contiguous() const { return empty() or ctrl_blk_ptr->segments.head == ctrl_blk_ptr->segments.tail; }

  /// Moves the bytes stored in different segments of the byte_buffer into first segment.
  [[nodiscard]] bool linearize();

  /// Set byte_buffer length. Note: It doesn't initialize newly created bytes.
  [[nodiscard]] bool resize(size_t new_sz);

  /// Returns a non-owning list of segments that compose the byte_buffer.
  byte_buffer_segment_span_range segments()
  {
    return {ctrl_blk_ptr != nullptr ? ctrl_blk_ptr->segments.head : nullptr, 0, length()};
  }
  const_byte_buffer_segment_span_range segments() const
  {
    return {ctrl_blk_ptr != nullptr ? ctrl_blk_ptr->segments.head : nullptr, 0, length()};
  }

  /// \brief Equality comparison between byte buffer view and another range.
  template <typename R>
  friend bool operator==(const byte_buffer& lhs, const R& r)
  {
    return detail::compare_byte_buffer_range(lhs, r);
  }
  template <typename T, std::enable_if_t<std::is_convertible_v<T, span<const uint8_t>>, int> = 0>
  friend bool operator==(const T& r, const byte_buffer& rhs)
  {
    return detail::compare_byte_buffer_range(rhs, r);
  }
  template <typename T>
  friend bool operator!=(const byte_buffer& lhs, const T& r)
  {
    return !(lhs == r);
  }
  template <typename T, std::enable_if_t<std::is_convertible_v<T, span<const uint8_t>>, int> = 0>
  friend bool operator!=(const T& r, const byte_buffer& rhs)
  {
    return !(rhs == r);
  }

private:
  bool has_ctrl_block() const { return ctrl_blk_ptr != nullptr; }

  [[nodiscard]] bool default_construct_unsafe(byte_buffer_memory_resource& segment_pool, unsigned sz_hint);

  [[nodiscard]] bool append(span<const uint8_t> bytes, byte_buffer_memory_resource& segment_pool);

  [[nodiscard]] node_t* add_head_segment(size_t headroom, byte_buffer_memory_resource& segment_pool, size_t sz_hint);

  [[nodiscard]] node_t* create_segment(size_t headroom, size_t sz_hint);

  [[nodiscard]] bool append_segment(size_t headroom_suggestion, size_t sz_hint);

  [[nodiscard]] bool prepend_segment(size_t headroom_suggestion, size_t sz_hint);

  /// \brief Removes last segment of the byte_buffer.
  /// Note: This operation is O(N), as it requires recomputing the tail.
  void pop_last_segment();

  intrusive_ptr<control_block> ctrl_blk_ptr;
};

/// \brief This class represents a sub-interval or make_slice of a potentially larger byte_buffer.
///
/// Like byte_buffer and byte_buffer_view, the represented bytes by this class are not contiguous in memory.
/// Contrarily to byte_buffer_view, this class retains shared ownership of the segments held by the byte_buffer which it
/// references.
/// Due to the shared ownership model, the usage of this class may involve additional overhead associated with reference
/// counting, which does not take place when using byte_buffer_view.
class byte_buffer_slice
{
public:
  using value_type     = byte_buffer_view::value_type;
  using iterator       = byte_buffer_view::iterator;
  using const_iterator = byte_buffer_view::const_iterator;

  /// Constructs an empty byte_buffer_slice.
  byte_buffer_slice() = default;

  /// Copy-construction of a byte_buffer_slice is a shallow copy of the underlying byte_buffer.
  explicit byte_buffer_slice(const byte_buffer_slice&) noexcept = default;

  byte_buffer_slice(byte_buffer_slice&&) noexcept = default;

  /// Creates a byte buffer slice from a span of bytes.
  static expected<byte_buffer_slice> create(span<const uint8_t> bytes)
  {
    auto buf = byte_buffer::create(bytes);
    if (not buf) {
      return make_unexpected(default_error_t{});
    }
    return byte_buffer_slice(std::move(buf.value()));
  }

  /// Creates a byte buffer slice from a list of bytes.
  static expected<byte_buffer_slice> create(std::initializer_list<uint8_t> bytes)
  {
    auto buf = byte_buffer::create(bytes);
    if (not buf) {
      return make_unexpected(default_error_t{});
    }
    return byte_buffer_slice(std::move(buf.value()));
  }

  /// Conversion from byte_buffer to byte_buffer_slice via move.
  byte_buffer_slice(byte_buffer&& buf_) : buf(std::move(buf_)), sliced_view(buf) {}

  /// Conversion from byte_buffer to byte_buffer_slice via shallow copy.
  byte_buffer_slice(const byte_buffer& buf_) : buf(buf_.copy()), sliced_view(buf) {}

  byte_buffer_slice(const byte_buffer& buf_, size_t offset, size_t length) :
    buf(buf_.copy()), sliced_view(buf, offset, length)
  {
  }

  /// Conversion from a [start, end) interval of a byte_buffer to a byte_buffer_slice.
  byte_buffer_slice(byte_buffer&& buf_, size_t offset, size_t length) :
    buf(std::move(buf_)), sliced_view(buf, offset, length)
  {
  }

  byte_buffer_slice(const byte_buffer& buf_, byte_buffer_view view) : buf(buf_.copy()), sliced_view(view)
  {
    srsran_sanity_check(view.begin() - byte_buffer_view{buf}.begin() < (int)buf.length(),
                        "byte_buffer_view is not part of the owned byte_buffer");
  }

  byte_buffer_slice(byte_buffer&& buf_, byte_buffer_view view) : buf(std::move(buf_)), sliced_view(view)
  {
    srsran_sanity_check(view.begin() - byte_buffer_view{buf}.begin() < (int)buf.length(),
                        "byte_buffer_view is not part of the owned byte_buffer");
  }

  /// Copy assignment is disabled. Use std::move or .copy() instead
  byte_buffer_slice& operator=(const byte_buffer_slice&) noexcept = delete;

  /// Move assignment of byte_buffer_slice. It avoids unnecessary reference counting increment.
  byte_buffer_slice& operator=(byte_buffer_slice&& other) noexcept = default;

  /// Performs a shallow copy. Nested segment reference counter is incremented.
  byte_buffer_slice copy() const { return byte_buffer_slice{*this}; }

  void clear()
  {
    buf.clear();
    sliced_view = {};
  }

  /// Converts to non-owning byte buffer view.
  byte_buffer_view view() const { return sliced_view; }

  explicit operator byte_buffer_view() const { return sliced_view; }

  /// Returns another owning sub-view with dimensions specified in arguments.
  byte_buffer_slice make_slice(size_t offset, size_t size) const
  {
    srsran_assert(offset + size <= length(), "Invalid view dimensions.");
    return {buf, sliced_view.view(offset, size)};
  }

  /// Advances slice by provided offset. The length of the slice gets automatically reduced by the provided offset.
  byte_buffer_slice& advance(size_t offset)
  {
    sliced_view = byte_buffer_view{sliced_view.begin() + offset, sliced_view.end()};
    return *this;
  }

  bool   empty() const { return sliced_view.empty(); }
  size_t length() const { return sliced_view.length(); }

  const uint8_t& operator[](size_t idx) const { return sliced_view[idx]; }

  iterator       begin() { return sliced_view.begin(); }
  const_iterator begin() const { return sliced_view.begin(); }
  iterator       end() { return sliced_view.end(); }
  const_iterator end() const { return sliced_view.end(); }

  /// Returns a non-owning list of segments that compose the byte_buffer.
  const_byte_buffer_segment_span_range segments() const { return sliced_view.segments(); }

  /// \brief Equality comparison between byte buffer slice and another range.
  template <typename T>
  friend bool operator==(const byte_buffer_slice& lhs, const T& r)
  {
    return detail::compare_byte_buffer_range(lhs, r);
  }
  template <typename T, std::enable_if_t<not is_byte_buffer_range<T>::value, int> = 0>
  friend bool operator==(const T& r, const byte_buffer_slice& rhs)
  {
    return detail::compare_byte_buffer_range(rhs, r);
  }
  template <typename T>
  friend bool operator!=(const byte_buffer_slice& lhs, const T& r)
  {
    return not(lhs == r);
  }
  template <typename T, std::enable_if_t<not is_byte_buffer_range<T>::value, int> = 0>
  friend bool operator!=(const T& r, const byte_buffer_slice& rhs)
  {
    return not(rhs == r);
  }

private:
  byte_buffer      buf;
  byte_buffer_view sliced_view;
};

inline bool byte_buffer::append(const byte_buffer_slice& slice)
{
  return append(slice.view());
}

/// Used to read a range of bytes stored in a byte_buffer.
class byte_buffer_reader : private byte_buffer_view
{
public:
  using byte_buffer_view::begin;
  using byte_buffer_view::byte_buffer_view;
  using byte_buffer_view::empty;
  using byte_buffer_view::end;
  using byte_buffer_view::length;

  byte_buffer_reader(const byte_buffer_view& other) : byte_buffer_view(other) {}

  /// Obtain a range view to the bytes pointed by the reader.
  byte_buffer_view view() const { return {it, it_end}; }

  const uint8_t& operator*() const { return *it; }

  const_iterator operator++() { return ++it; }

  /// Advance reader by offset bytes. Returns an iterator to new position.
  const_iterator operator+=(size_t offset) { return it += offset; }

  /// Advance offset bytes and returns view to skipped bytes.
  byte_buffer_view split_and_advance(size_t offset)
  {
    auto prev_it = it;
    it += offset;
    return {prev_it, it};
  }
};

/// Used to write into a range of bytes stored in a byte_buffer.
class byte_buffer_writer
{
public:
  byte_buffer_writer(byte_buffer& other) : buffer(&other) {}

  /// Obtain a range view to the bytes pointed by the reader.
  byte_buffer_view view() const { return *buffer; }

  /// Appends bytes.
  [[nodiscard]] bool append(byte_buffer_view bytes) { return buffer->append(bytes); }

  /// Appends initializer list of bytes.
  [[nodiscard]] bool append(const std::initializer_list<uint8_t>& bytes)
  {
    return buffer->append(span<const uint8_t>{bytes.begin(), bytes.size()});
  }

  /// Appends span of bytes.
  [[nodiscard]] bool append(span<const uint8_t> bytes) { return buffer->append(bytes); }

  /// Appends a single byte.
  [[nodiscard]] bool append(uint8_t byte) { return buffer->append(byte); }

  /// Appends the specified amount of zeros.
  [[nodiscard]] bool append_zeros(size_t nof_zeros);

  /// Checks last appended byte.
  uint8_t& back() { return buffer->back(); }

  /// Number of bytes in the byte_buffer.
  size_t length() const { return buffer->length(); }

  /// Checks whether any byte has been written.
  bool empty() const { return buffer->empty(); }

private:
  byte_buffer* buffer;
};

/// Converts a string of hexadecimal digits (e.g. "01FA02") to a byte buffer.
expected<byte_buffer> make_byte_buffer(const std::string& hex_str);

/// Performs a segment-wise copy of the byte_buffer into a span<uint8_t> object.
/// The length is limited by the length of src and dst, whichever is smaller.
///
/// \param src Source byte_buffer.
/// \param dst Destination span<uint8_t>.
/// \return Number of bytes copied.
template <typename ByteBufferType>
inline size_t copy_segments(const ByteBufferType& src, span<uint8_t> dst)
{
  size_t bytes_copied    = 0;
  size_t bytes_remaining = std::min(src.length(), dst.size_bytes());
  for (const auto& src_segment : src.segments()) {
    size_t        block_size  = std::min(bytes_remaining, src_segment.size());
    span<uint8_t> dst_segment = dst.subspan(bytes_copied, block_size);
    std::copy(src_segment.begin(), src_segment.begin() + block_size, dst_segment.begin());
    bytes_copied += block_size;
    bytes_remaining -= block_size;
    if (bytes_remaining == 0) {
      break;
    }
  }
  return bytes_copied;
}

/// Provides a contiguous view of a byte_buffer.
/// It is bound to the lifetime of the byte_buffer \p src and the temporary buffer \p tmp_mem, whichever is shorter.
/// Important: \p tmp_mem must be at least as large as \p src.
///
/// The memory used for the result and the complexity depends on whether \p src is contiguous or not:
/// - Contiguous: Result is a view of the first and only segment of \p src.
/// - Non-Contiguous: Result is a view of \p tmp_mem in which all segments of \p src are copied.
///
/// \param src Source byte_buffer.
/// \param tmp_mem Temporary memory for a possible copy. Must be at least as large as \p src.
/// \return A contiguous view of the byte_buffer
span<const uint8_t> to_span(const byte_buffer& src, span<uint8_t> tmp_mem);

} // namespace srsran

namespace fmt {

/// \brief Custom formatter for byte_buffer.
template <>
struct formatter<srsran::byte_buffer> {
  enum { hexadecimal, binary } mode = hexadecimal;

  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    auto it = ctx.begin();
    while (it != ctx.end() and *it != '}') {
      if (*it == 'b') {
        mode = binary;
      }
      ++it;
    }
    return it;
  }

  template <typename FormatContext>
  auto format(const srsran::byte_buffer& buf, FormatContext& ctx) const
  {
    if (mode == hexadecimal) {
      return format_to(ctx.out(), "{:0>2x}", fmt::join(buf.begin(), buf.end(), " "));
    }
    return format_to(ctx.out(), "{:0>8b}", fmt::join(buf.begin(), buf.end(), " "));
  }
};

/// \brief Custom formatter for byte_buffer_slice.
template <>
struct formatter<srsran::byte_buffer_slice> : public formatter<srsran::byte_buffer_view> {
  template <typename FormatContext>
  auto format(const srsran::byte_buffer_slice& buf, FormatContext& ctx) const
  {
    return formatter<srsran::byte_buffer_view>::format(buf.view(), ctx);
  }
};

} // namespace fmt
