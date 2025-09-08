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

#ifdef ENABLE_TSAN
#include <sanitizer/tsan_interface.h>
#endif

#include "cameron314/concurrentqueue.h"
#include "srsran/adt/static_vector.h"
#include "srsran/support/error_handling.h"
#include "srsran/support/memory_pool/memory_block_list.h"
#include "srsran/support/srsran_assert.h"
#include <mutex>
#include <thread>
#include <vector>

namespace srsran {

/**
 * Concurrent memory pool of memory blocks of equal size. This pool is thread-safe.
 * Each worker keeps a separate thread-local memory block cache that it uses for fast, non-contended allocation and
 * deallocation. When accessing a thread-local cache, no locks are required.
 *
 * When the local cache gets depleted, the worker tries to obtain a batch of segments from a central memory block cache.
 * If the central cache is also depleted, the allocation fails.
 *
 * Since there is no stealing of segments between workers' local caches, it is possible that a worker cannot allocate
 * while another worker still has blocks in its own cache. To minimize the impact of this event, an upper bound is
 * placed on a worker local cache size. Once a worker reaches that upper bound, it sends half of its stored blocks to
 * the central cache.
 *
 * Note1: Notice that the same memory block might be allocated and deallocated in different workers. So, it can happen
 * that a worker is allocating many blocks and another worker just deallocating. The latter worker has to keep
 * returning blocks back to the central cache every time its local cache grows beyond a given upper bound.
 *
 * Note2: Taking into account the usage of thread_local, this class is made a singleton. To be able to instantiate
 *       different pools, the user should use different IdTag types.
 *
 * Note3: No considerations were made regarding false sharing between workers. It is assumed that the blocks are big
 *        enough to fill a cache line.
 *
 * \tparam IdTag We use a ID type tag to be able to intantiate different pool objects.
 * \tparam DebugSanitizeAddress when set to true, the pool verifies that the addresses allocated and deallocated are
 * valid.
 */
template <typename IdTag, bool DebugSanitizeAddress = false>
class fixed_size_memory_block_pool
{
  /// The number of blocks in batch that the worker can steal from the central cache.
  static constexpr size_t block_batch_size = 32U;

  /// The number of batches of blocks that a worker can store in its own thread for non-contended access.
  static constexpr size_t MAX_LOCAL_BATCH_CAPACITY = 64U;

  /// A batch of memory blocks that is exchanged in bulk between the central and local caches.
  using memory_block_batch = free_memory_block_list;

  /// Thread-local cache that stores a list of batches of memory blocks.
  using local_cache_type = static_vector<memory_block_batch, MAX_LOCAL_BATCH_CAPACITY>;

  // Given that we use the MPMC queue in https://github.com/cameron314/concurrentqueue, we have to over-dimension it
  // to account the potential number of producers. The way to exactly over-dimension this queue is inconvenient, so
  // we just try to conservatively ensure it can accommodate up to 32 producers for a block size of 32. If this is
  // not enough, the queue will resize itself and malloc in the process.
  static const size_t OVER_DIM_CENTRAL_CACHE = 2 * 32 * 32;

  /// Ctor of the memory pool. It is set as private because the class works as a singleton.
  explicit fixed_size_memory_block_pool(size_t nof_blocks_, size_t memory_block_size_) :
    // Make sure that there are no gaps between blocks when they are allocated as paret of a single array.
    mblock_size(align_next(memory_block_size_, alignof(std::max_align_t))),
    // Make sure all batches are filled with block_batch_size blocks.
    nof_blocks(ceil(nof_blocks_ / (double)block_batch_size) * block_batch_size),
    // Calculate the maximum number of batches that can be stored in the local cache.
    max_local_batches(
        std::max(std::min((size_t)MAX_LOCAL_BATCH_CAPACITY, static_cast<size_t>(nof_blocks / block_batch_size / 32U)),
                 static_cast<size_t>(2U))),
    // Allocate the required memory for the given number of segments and segment size.
    allocated_memory(mblock_size * nof_blocks),
    // Pre-reserve space in the central cache to hold all batches and avoid reallocations.
    central_mem_cache(nof_total_batches() + OVER_DIM_CENTRAL_CACHE)
  {
    srsran_assert(nof_blocks > max_local_cache_size(),
                  "The number of segments in the pool must be much larger than the thread cache size ({} <= {})",
                  nof_blocks,
                  max_local_cache_size());
    srsran_assert(mblock_size > free_memory_block_list::min_memory_block_align(),
                  "Segment size is too small ({} <= {})",
                  mblock_size,
                  free_memory_block_list::min_memory_block_align());

    // Push all memory blocks to the central cache in batches.
    const unsigned nof_batches = nof_total_batches();
    for (unsigned i = 0; i != nof_batches; ++i) {
      free_memory_block_list batch;
      for (unsigned j = 0; j != block_batch_size; ++j) {
        batch.push(allocated_memory.data() + (i * block_batch_size + j) * mblock_size);
      }
      report_fatal_error_if_not(central_mem_cache.enqueue(batch), "Failed to push batch to central cache");
    }
  }

public:
  using pool_type = fixed_size_memory_block_pool<IdTag, DebugSanitizeAddress>;

  fixed_size_memory_block_pool(const fixed_size_memory_block_pool&)            = delete;
  fixed_size_memory_block_pool(fixed_size_memory_block_pool&&)                 = delete;
  fixed_size_memory_block_pool& operator=(const fixed_size_memory_block_pool&) = delete;
  fixed_size_memory_block_pool& operator=(fixed_size_memory_block_pool&&)      = delete;

  ~fixed_size_memory_block_pool()
  {
    if constexpr (DebugSanitizeAddress) {
      unsigned                           rem_batches = nof_total_batches();
      free_memory_block_list             list;
      std::unordered_map<long int, bool> addresses;
      for (unsigned i = 0; i < rem_batches; i++) {
        report_fatal_error_if_not(central_mem_cache.try_dequeue(list), "segments were lost {} < {}", i, rem_batches);
        for (unsigned j = 0; j < block_batch_size; j++) {
          void* p = list.try_pop();
          report_fatal_error_if_not(p != nullptr, "lost segment {} < {}", j, block_batch_size);
          report_fatal_error_if_not(addresses.find((long int)p) == addresses.end(), "repeated segment detected");
          addresses.insert(std::make_pair((long int)p, true));
        }
      }
      report_fatal_error_if_not(not central_mem_cache.try_dequeue(list), "more batches than when initialized");
    }
  }

  /// \brief Get instance of a memory pool singleton.
  static pool_type& get_instance(size_t nof_blocks = 0, size_t mem_block_size = 0)
  {
    static pool_type& pool = *get_instance_ptr(nof_blocks, mem_block_size);
    return pool;
  }

  /// Memory block size in bytes.
  size_t memory_block_size() const { return mblock_size; }

  /// Number of memory blocks contained in this memory pool.
  size_t nof_memory_blocks() const { return nof_blocks; }

  /// Maximum number of blocks that can be stored in the thread-local memory block cache.
  size_t max_local_cache_size() const { return max_local_batches * block_batch_size; }

  /// Allocate a node from the memory pool with the maximum size.
  void* allocate_node() noexcept { return allocate_node(memory_block_size()); }

  /// Allocate a node from the memory pool with the provided size.
  void* allocate_node(size_t sz) noexcept
  {
    srsran_assert(sz <= mblock_size, "Allocated node size={} exceeds max object size={}", sz, mblock_size);
    worker_ctxt* w_ctx = get_worker_cache();

    // Attempt memory block pop from local cache.
    void* node = nullptr;
    while (not w_ctx->local_cache.empty()) {
      node = w_ctx->local_cache.back().try_pop();
      if (node != nullptr) {
        validate_node_address(node);
        return node;
      }
      w_ctx->local_cache.pop_back();
    }

    // Local cache is empty. Attempt memory block pop from central cache.
    free_memory_block_list batch;
    if (central_mem_cache.try_dequeue(w_ctx->consumer_token, batch)) {
      w_ctx->local_cache.push_back(batch);
#ifdef ENABLE_TSAN
      __tsan_acquire((void*)w_ctx->local_cache.back().head);
#endif
      node = w_ctx->local_cache.back().try_pop();
      validate_node_address(node);
    }

    return node;
  }

  /// Deallocate node by returning it back to the memory pool.
  void deallocate_node(void* p)
  {
    srsran_assert(p != nullptr, "Deallocated nodes must have valid address");
    validate_node_address(p);

    worker_ctxt* w_ctx = get_worker_cache();

    // Verify if new batch needs to be created in local cache.
    if (w_ctx->local_cache.empty() or w_ctx->local_cache.back().size() >= block_batch_size) {
      w_ctx->local_cache.emplace_back();
    }

    // Push block to local cache.
    w_ctx->local_cache.back().push(p);
#ifdef ENABLE_TSAN
    __tsan_release(p);
#endif

    if (w_ctx->local_cache.size() >= max_local_batches and w_ctx->local_cache.back().size() >= block_batch_size) {
      // Local cache is full. Rebalance by sending batches of blocks to central cache.
      // We leave one batch in the local cache.
      for (unsigned i = 0; i != max_local_batches - 1; ++i) {
        report_fatal_error_if_not(central_mem_cache.enqueue(w_ctx->producer_token, w_ctx->local_cache.back()),
                                  "Failed to push allocated batch back to central cache");
        w_ctx->local_cache.pop_back();
      }
    }
  }

  void print_all_buffers()
  {
    fmt::print("There are {}/{} buffers in central memory block cache. This thread contains {} in its local cache.\n",
               central_mem_cache.size_approx() * block_batch_size,
               nof_memory_blocks(),
               get_local_cache_size());
  }

  /// Get central cache current size in number of memory blocks.
  size_t get_central_cache_approx_size() const { return central_mem_cache.size_approx() * block_batch_size; }

  /// Get thread local cache current size in number of memory blocks.
  size_t get_local_cache_size()
  {
    auto* w = get_worker_cache();
    if (w->local_cache.empty()) {
      return 0;
    }
    return (w->local_cache.size() - 1) * block_batch_size + w->local_cache.back().size();
  }

  /// Check if the memory pool owns the given memory segment.
  bool owns_segment(void* segment) const
  {
    uint8_t* ptr = static_cast<uint8_t*>(segment);
    return ptr >= allocated_memory.data() and ptr < allocated_memory.data() + allocated_memory.size();
  }

  /// \brief Initialize worker cache for caller thread.
  /// This method is useful to avoid the overhead of thread_local initialization in critical paths.
  void init_worker_cache()
  {
    if (void* p = allocate_node(mblock_size)) {
      deallocate_node(p);
    }
  }

private:
  static std::shared_ptr<pool_type> get_instance_ptr(size_t nof_blocks = 0, size_t mem_block_size = 0)
  {
    // We use a shared_ptr to keep the pool alive, because we have no control over the order of destruction of workers
    // and the pool. e.g. this static pool object in this function could be destroyed before the worker_ctxt objects.
    static std::shared_ptr<pool_type> pool(new pool_type(nof_blocks, mem_block_size));
    return pool;
  }

  struct worker_ctxt {
    /// Shared ownership of the pool.
    std::shared_ptr<pool_type> parent;
    /// Thread ID of the worker.
    std::thread::id id;
    /// Thread-local cache of memory blocks.
    local_cache_type local_cache;
    /// Producer Token for fast enqueueing to the central cache.
    moodycamel::ProducerToken producer_token;
    /// Consumer Token for fast dequeueing to the central cache.
    moodycamel::ConsumerToken consumer_token;

    worker_ctxt() :
      parent(pool_type::get_instance_ptr()),
      id(std::this_thread::get_id()),
      producer_token(parent->central_mem_cache),
      consumer_token(parent->central_mem_cache)
    {
    }
    ~worker_ctxt()
    {
      while (not local_cache.empty()) {
        if (local_cache.back().size() < block_batch_size) {
          // Batch is incomplete. We combine it with any other existing incomplete batch.
          {
            std::lock_guard<std::mutex> lock(parent->incomplete_batch_mutex);
            while (not local_cache.back().empty()) {
              parent->incomplete_batch.push(local_cache.back().try_pop());
              if (parent->incomplete_batch.size() >= block_batch_size) {
                // Note: Central_mem_cache uses a queue that uses atomic_fences. TSAN doesn't deal well with atomic
                // fences, so we need to add a hint here.
#ifdef ENABLE_TSAN
                __tsan_release((void*)parent->incomplete_batch.head);
#endif
                // The incomplete batch is now complete and can be pushed to the central cache.
                report_error_if_not(parent->central_mem_cache.enqueue(producer_token, parent->incomplete_batch),
                                    "Failed to push blocks to central cache");
                parent->incomplete_batch.clear();
              }
            }
          }
          local_cache.pop_back();
          continue;
        }
        report_error_if_not(parent->central_mem_cache.enqueue(producer_token, local_cache.back()),
                            "Failed to push blocks back to central cache");
        local_cache.pop_back();
      }
    }
  };

  worker_ctxt* get_worker_cache()
  {
    thread_local worker_ctxt worker_cache{};
    return &worker_cache;
  }

  /// Number of batches of memory blocks stored in the pool.
  size_t nof_total_batches() const { return (nof_blocks + block_batch_size - 1) / block_batch_size; }

  void validate_node_address(void* node)
  {
#ifdef ASSERTS_ENABLED
    bool validation_enabled = true;
#else
    bool validation_enabled = DebugSanitizeAddress;
#endif
    if (validation_enabled and node != nullptr) {
      report_fatal_error_if_not(node >= allocated_memory.data() and
                                    node < allocated_memory.data() + allocated_memory.size(),
                                "Invalid memory block address");
      report_fatal_error_if_not((static_cast<uint8_t*>(node) - allocated_memory.data()) % mblock_size == 0,
                                "Misaligned memory block address");
    }
  }

  const size_t mblock_size;
  const size_t nof_blocks;
  const size_t max_local_batches;

  std::vector<uint8_t> allocated_memory;

  moodycamel::ConcurrentQueue<free_memory_block_list> central_mem_cache;

  // When workers get deleted, some local batches may be still incomplete. We collect them here to form full batches
  // when other workers get deleted as well.
  std::mutex             incomplete_batch_mutex;
  free_memory_block_list incomplete_batch;
};

} // namespace srsran
