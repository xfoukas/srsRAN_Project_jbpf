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

#include "rlc_bearer_logger.h"
#include "srsran/adt/spsc_queue.h"
#include "srsran/rlc/rlc_tx.h"
#include "fmt/std.h"

namespace srsran {

/// \brief Lockfree RLC SDU queue
///
/// This class is used as FIFO buffer for RLC SDUs from upper layers that shall be transmitted by the RLC Tx entity.
/// It implements SPSC (single producer single consumer) semantics, i.e. allows concurrent access by one upper-layer
/// thread and one lower-layer thread. Internally, this class wraps a lock-free SPSC queue to store the actual SDUs and
/// involves additional bookkeeping to support discard of SDUs and to track the amount of buffered SDUs and bytes.
///
/// From the perspective of the upper-layer thread this class provides methods to write RLC SDUs (with or without PDCP
/// SN) and to discard SDUs by their PDCP SN. SDUs without PDCP SN cannot be discarded.
/// SDUs that are marked as discarded remain in the internal queue until they are popped (and dropped) via \c read.
/// Writing to the queue fails if the internal queue is either full or the queue already contains an SDU (whether valid
/// or marked as discarded) with the same value for [PDCP_SN mod capacity].
///
/// From the perspective of the lower-layer thread it provides methods to read RLC SDUs and to query the total number of
/// buffered SDUs and bytes.
/// SDUs are read in the same order as they were written into the queue. There is no reordering by PDCP SN.
class rlc_sdu_queue_lockfree
{
public:
  explicit rlc_sdu_queue_lockfree(uint32_t capacity_, uint32_t byte_limit_, rlc_bearer_logger& logger_) :
    logger(logger_), capacity(capacity_), byte_limit(byte_limit_)
  {
    sdu_states = std::make_unique<std::atomic<uint32_t>[]>(capacity);
    sdu_sizes  = std::make_unique<std::atomic<size_t>[]>(capacity);

    for (uint32_t i = 0; i < capacity; i++) {
      sdu_states[i].store(STATE_FREE, std::memory_order_relaxed);
    }

    queue = std::make_unique<
        concurrent_queue<rlc_sdu, concurrent_queue_policy::lockfree_spsc, concurrent_queue_wait_policy::non_blocking>>(
        capacity);
  }

  /// \brief Writes an RLC SDU (with optional PDCP SN) to the queue, if possible.
  ///
  /// This function may be called by the upper-layer thread.
  ///
  /// The write fails (returns false) in the following cases:
  /// - The internal queue is full.
  /// - Another SDU with same value of [PDCP_SN mod capacity] exists (either valid or discarded) in the queue.
  /// - The new SDU makes the queue exceed a preconfigured limit of buffered bytes.
  ///
  /// \param sdu The RLC SDU that shall be written.
  /// \return True if the RLC SDU was successfully written to the queue, otherwise false.
  bool write(rlc_sdu sdu)
  {
    // first check if we do not exeed the byte limit
    state_t st = get_state();
    if (sdu.buf.length() + st.n_bytes >= byte_limit) {
      return false;
    }

    // if the SDU has a PDCP SN, first check the slot is available
    std::optional<uint32_t> pdcp_sn  = sdu.pdcp_sn;
    const size_t            sdu_size = sdu.buf.length();
    if (pdcp_sn.has_value()) {
      const uint32_t pdcp_sn_value = sdu.pdcp_sn.value();
      // load slot state (memory_order_acquire ensures sdu_size is written after this)
      uint32_t slot_state = sdu_states[pdcp_sn_value % capacity].load(std::memory_order_acquire);
      if (slot_state != STATE_FREE) {
        logger.log_debug("SDU queue failed to enqueue pdcp_sn={}. Slot holds pdcp_sn={}", pdcp_sn_value, slot_state);
        return false;
      }

      // slot is free, we can safely store the SDU size
      sdu_sizes[pdcp_sn_value % capacity].store(sdu_size, std::memory_order_relaxed);

      // allocate slot by writing the PDCP SN into it (memory_order_relaxed, don't care about order because of "release"
      // semantics of the push to queue)
      sdu_states[pdcp_sn_value % capacity].store(pdcp_sn_value, std::memory_order_relaxed);
    }

    // push SDU to queue
    bool pushed = queue->try_push(std::move(sdu));
    if (not pushed) {
      logger.log_debug("SDU queue failed to enqueue pdcp_sn={}. Queue is full", pdcp_sn);
      // if the SDU has a PDCP SN, release the slot (memory_order_relaxed, don't care about value of sdu_size)
      if (pdcp_sn.has_value()) {
        sdu_states[pdcp_sn.value() % capacity].store(STATE_FREE, std::memory_order_relaxed);
      }
      return false;
    }

    // update totals
    state_add(sdu_size);
    return true;
  }

  /// \brief Marks an RLC SDU as "discarded", if possible.
  ///
  /// This function may be called by the upper-layer thread.
  ///
  /// The function fails (returns false) in the following cases:
  /// - The SDU with the PDCP SN is already marked as discarded.
  /// - The SDU with the PDCP SN is not in the queue.
  ///
  /// In case of success, the number of SDUs/bytes will be updated immediately.
  /// However, the actual SDU remains in the internal queue until it is popped and finally discarded via \c read.
  /// Meanwhile, no further SDU with the same value of [PDCP_SN mod capacity] can be added to the queue.
  ///
  /// \param pdcp_sn The PDCP SN of the SDU that shall be discarded.
  /// \return True if the RLC SDU was successfully discarded, otherwise false.
  bool try_discard(uint32_t pdcp_sn)
  {
    uint32_t expected_state = pdcp_sn;
    uint32_t desired_state  = STATE_DISCARDED;
    // set slot state as "discarded" only if it holds the PDCP SN; otherwise load the existing state
    // - on success: memory_order_acquire ensures sdu_size is up to date
    // - on failure: memory_order_relaxed, don't care about value of sdu_size
    bool success = sdu_states[pdcp_sn % capacity].compare_exchange_strong(
        expected_state, desired_state, std::memory_order_acquire, std::memory_order_relaxed);
    uint32_t sdu_size = sdu_sizes[pdcp_sn % capacity].load(std::memory_order_relaxed);

    if (not success) {
      switch (expected_state) {
        case STATE_DISCARDED:
          logger.log_debug("SDU queue cannot discard pdcp_sn={}. Slot is already discarded", pdcp_sn);
          break;
        case STATE_FREE:
          logger.log_debug("SDU queue cannot discard pdcp_sn={}. Slot is already free", pdcp_sn);
          break;
        default:
          logger.log_debug("SDU queue cannot discard pdcp_sn={}. Slot holds pdcp_sn={}", pdcp_sn, expected_state);
          break;
      }
      return false;
    }

    // update totals
    state_sub(sdu_size);
    return true;
  }

  /// \brief Reads an RLC SDU (with optional PDCP SN) from the queue, if possible.
  ///
  /// This function may be called by the lower-layer thread.
  ///
  /// The read fails (returns false) in the following cases:
  /// - The internal queue only contains SDUs that are marked as discarded.
  /// - The internal queue is empty.
  ///
  /// Each call of this function pops and drops SDUs that are marked as discarded from the internal queue in a loop
  /// until a valid SDU is popped (return true) or the queue is empty (returns false).
  ///
  /// \param sdu Reference to a \c rlc_sdu object that will be filled with the read RLC SDU.
  /// \return True if an RLC SDU was successfully read from the queue, otherwise false.
  bool read(rlc_sdu& sdu)
  {
    bool sdu_is_valid = true;
    do {
      // first try to pop front (SDU can still get discarded from upper layers)
      bool popped = queue->try_pop(sdu);
      if (not popped) {
        // queue is empty
        return false;
      }

      if (sdu.pdcp_sn.has_value()) {
        // Check if the SDU is still valid (i.e. the PDCP SN was not already discarded) and release the slot
        const uint32_t pdcp_sn = sdu.pdcp_sn.value();
        sdu_is_valid           = check_and_release(pdcp_sn); // this also updates totals
      } else {
        // SDUs without PDCP SN are alway valid as they can't be discarded
        sdu_is_valid = true;

        // update totals
        state_sub(sdu.buf.length());
      }
      // try again if SDU is not valid
    } while (not sdu_is_valid);
    return true;
  }

  rlc_sdu* front()
  {
    rlc_sdu* front_sdu;
    bool     sdu_is_valid = true;
    do {
      // first try to access front (SDU can still get discarded from upper layers)
      front_sdu = queue->front();
      if (front_sdu == nullptr) {
        // queue is empty
        return nullptr;
      }

      if (front_sdu->pdcp_sn.has_value()) {
        // Check if the SDU is still valid (i.e. the PDCP SN was not already discarded)
        const uint32_t pdcp_sn = front_sdu->pdcp_sn.value();
        sdu_is_valid           = check(pdcp_sn);

        if (not sdu_is_valid) {
          // Pop the invalid SDU that is marked as discarded
          rlc_sdu pop_sdu;
          bool    popped = queue->try_pop(pop_sdu);
          if (not popped) {
            logger.log_error("Failed to pop SDU from SDU queue with front marked as invalid. pdcp_sn={}", pdcp_sn);
            return nullptr;
          }
          if (not pop_sdu.pdcp_sn.has_value() || pop_sdu.pdcp_sn.value() != pdcp_sn) {
            logger.log_error(
                "Popped unexpected SDU from SDU queue with front marked as invalid. pdcp_sn={} pop_sdu.pdcp_sn={}",
                pdcp_sn,
                pop_sdu.pdcp_sn);
            return nullptr;
          }

          // Release the slot using the check_and_release function; this must return false (i.e. the SDU was invalid)
          sdu_is_valid = check_and_release(pdcp_sn); // this also updates totals
          if (sdu_is_valid) {
            logger.log_error(
                "Unexpected result of check_and_release for popped front that was marked as invalid. pdcp_sn={}",
                pdcp_sn);
            return nullptr;
          }
        }
      } else {
        // SDUs without PDCP SN are always valid as they can't be discarded
        sdu_is_valid = true;
      }
      // try again if SDU is not valid
    } while (not sdu_is_valid);

    return front_sdu;
  }

  /// \brief Container for return value of \c get_state function.
  struct state_t {
    uint32_t n_sdus;  ///< Number of buffered SDUs that are not marked as discarded.
    uint32_t n_bytes; ///< Number of buffered bytes that are not marked as discarded.
  };

  /// \brief Reads the state of the queue, i.e. number of buffered SDUs and bytes that are not marked as discarded.
  ///
  /// This function may be called by any thread.
  ///
  /// \return Current state of the queue.
  state_t get_state() const
  {
    uint64_t packed = state.load(std::memory_order_relaxed);
    state_t  result;
    result.n_bytes = packed & 0xffffffff;
    result.n_sdus  = packed >> 32;
    return result;
  }

  /// \brief Checks if the internal queue is empty.
  ///
  /// This function may be called by any thread.
  ///
  /// \return True if the internal queue is empty, otherwise false.
  bool is_empty() { return queue->empty(); }

  /// \brief Checks if the internal queue is full.
  ///
  /// This function may be called by any thread.
  ///
  /// \return True if the internal queue is full, otherwise false.
  bool is_full() { return queue->size() >= capacity; }

private:
  /// \brief Checks if the RLC SDU with a PDCP SN is valid (i.e. not marked as discarded) and marks the respective slot
  /// as free.
  ///
  /// This private function may be called by \c read by the lower-layer thread.
  ///
  /// The function fails (returns false) in the following cases:
  /// - The SDU with the PDCP SN is already marked as discarded.
  /// - The SDU with the PDCP SN is not in the queue.
  ///
  /// The number of SDUs/bytes will be updated unless the SDU is marked as discarded (in that case it was already
  /// updated by \c try_discard).
  ///
  /// \param pdcp_sn The PDCP SN of the SDU that shall be checked and released.
  /// \return True if the RLC SDU with given PDCP SN is valid, otherwise false.
  bool check_and_release(uint32_t pdcp_sn)
  {
    uint32_t sdu_size = sdu_sizes[pdcp_sn % capacity].load(std::memory_order_relaxed);
    // free the slot (memory_order_release ensures sdu_size is read before the state is set to free)
    uint32_t old_state = sdu_states[pdcp_sn % capacity].exchange(STATE_FREE, std::memory_order_release);

    bool sdu_is_valid;
    if (old_state == pdcp_sn) {
      sdu_is_valid = true;
      logger.log_debug("SDU queue released valid pdcp_sn={}", pdcp_sn);
    }
    if (old_state != pdcp_sn) {
      sdu_is_valid = false;
      switch (old_state) {
        case STATE_DISCARDED:
          logger.log_debug("SDU queue released discarded pdcp_sn={}", pdcp_sn);
          return false; // totals were already updated on discard
        case STATE_FREE:
          logger.log_error("SDU queue error: Invalid release of pdcp_sn={}. Slot was already free", pdcp_sn);
          break;
        default:
          logger.log_error("SDU queue error: Invalid release of pdcp_sn={}. Slot holds pdcp_sn={}", pdcp_sn, old_state);
          break;
      }
    }

    // update totals
    state_sub(sdu_size);
    return sdu_is_valid;
  }

  /// \brief Checks if the RLC SDU with a PDCP SN is valid (i.e. not marked as discarded)
  ///
  /// This private function may be called by \c read by the lower-layer thread.
  ///
  /// The function fails (returns false) in the following cases:
  /// - The SDU with the PDCP SN is already marked as discarded.
  /// - The SDU with the PDCP SN is not in the queue.
  ///
  /// \param pdcp_sn The PDCP SN of the SDU that shall be checked.
  /// \return True if the RLC SDU with given PDCP SN is valid, otherwise false.
  bool check(uint32_t pdcp_sn)
  {
    uint32_t sdu_state = sdu_states[pdcp_sn % capacity].load(std::memory_order_relaxed);
    return sdu_state == pdcp_sn;
  }

  static constexpr uint32_t STATE_FREE      = 0xffffffff; ///< Sentinel value to mark a slot as free.
  static constexpr uint32_t STATE_DISCARDED = 0xfffffffe; ///< Sentinel value to mark a slot as discarded.

  rlc_bearer_logger& logger;

  const uint32_t capacity;
  const uint32_t byte_limit;

  /// Combined atomic state of the queue reflecting the number of SDUs and the number of bytes.
  /// Upper 32 bit: n_sdus; Lower 32 bit: n_bytes
  std::atomic<uint64_t> state = {0};

  std::unique_ptr<
      concurrent_queue<rlc_sdu, concurrent_queue_policy::lockfree_spsc, concurrent_queue_wait_policy::non_blocking>>
      queue;

  std::unique_ptr<std::atomic<uint32_t>[]> sdu_states;
  std::unique_ptr<std::atomic<size_t>[]>   sdu_sizes;

  /// \brief Adds one SDU of given size to the atomic queue state.
  /// \param sdu_size The size of the SDU.
  void state_add(uint32_t sdu_size)
  {
    uint64_t state_change = static_cast<uint64_t>(1U) << 32U | sdu_size;
    state.fetch_add(state_change, std::memory_order_relaxed);
  }

  /// \brief Subtracts one SDU of given size from the atomic queue state.
  /// \param sdu_size The size of the SDU.
  void state_sub(uint32_t sdu_size)
  {
    uint64_t state_change = static_cast<uint64_t>(1U) << 32U | sdu_size;
    state.fetch_sub(state_change, std::memory_order_relaxed);
  }
};

} // namespace srsran

namespace fmt {
template <>
struct formatter<srsran::rlc_sdu_queue_lockfree::state_t> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const srsran::rlc_sdu_queue_lockfree::state_t& state, FormatContext& ctx) const
  {
    return format_to(ctx.out(), "queued_sdus={} queued_bytes={}", state.n_sdus, state.n_bytes);
  }
};

} // namespace fmt
