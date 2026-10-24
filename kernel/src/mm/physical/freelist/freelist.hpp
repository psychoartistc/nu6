#pragma once

#include <mm/physical/page_allocator.hpp>
#include <atomic>
#include <std/lock_guard.hpp>

namespace mm {
class freelist_allocator {
private:
  std::uintptr_t m_head{0x0};
  std::uintptr_t m_tail{0x0};

  // seq_cst
  std::atomic<std::uint64_t> m_totalFrames;

  // acq, rel
  std::atomic<std::uint64_t> m_freeFrames;

public:
  void load();

  std::expected<std::uintptr_t, mm::page_allocator::error_code> pop();
  bool push();

  std::uint64_t total_frames() const {
    return m_totalFrames.load(std::memory_order_acquire);
  }
};

} // namespace mm
