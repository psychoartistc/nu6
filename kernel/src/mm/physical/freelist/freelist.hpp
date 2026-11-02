#pragma once

#include <mm/physical/page_allocator.hpp>
#include <std/embedded_freelist.hpp>
#include <limine/requests.hpp>
#include <mm/common.hpp>
#include <utils/fmt.hpp>
#include <utils/math.hpp>
#include <logging/logging.hpp>
#include <atomic>
#include <expected>

namespace mm {
class freelist_allocator {
private:
  using error_code = page_allocator::error_code;

private:
  logging::named_logger m_logger{"freelist_allocator"};
  std::embedded_freelist m_freelist{};
  std::atomic<std::uint64_t> m_totalFrames{0};
  std::atomic<std::uint64_t> m_freeFrames{0};

public:
  bool reclaim_type(std::uint64_t type);

  std::expected<std::uintptr_t, page_allocator::error_code> pop();

  bool push(std::uintptr_t a);

  std::atomic<std::uint64_t> total_frames() const {
    return m_totalFrames.load(std::memory_order_relaxed);
  }

  std::atomic<std::uint64_t> free_frames() const {
    return m_freeFrames.load(std::memory_order_relaxed);
  }
};
} // namespace mm
