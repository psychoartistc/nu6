#pragma once

#include <mm/physical/page_allocator.hpp>
#include <atomic>
#include <std/lock_guard.hpp>
#include <logging/logging.hpp>

namespace mm {
class freelist_allocator {
private:
  logging::named_logger m_logger{"freelist"};
  std::uintptr_t m_head{0x0};
  std::uintptr_t m_tail{0x0};

  // seq_cst
  std::atomic<std::uint64_t> m_totalFrames{0};

  // relaxed
  std::atomic<std::uint64_t> m_freeFrames{0};

  std::spinlock_mutex m_mutex;

  // seq_cst
  std::atomic<bool> m_bNotEmpty = false;

public:
  bool reclaim_type(std::uint64_t type);

  std::expected<std::uintptr_t, mm::page_allocator::error_code> pop();

  bool push(std::uintptr_t page);

  std::uint64_t free_frames() const {
    return m_freeFrames.load(std::memory_order_relaxed);
  }

  std::uint64_t total_frames() const { return m_totalFrames.load(); }
};

} // namespace mm
