#pragma once

#include <optional>
#include <arch/irq/common.hpp>
#include <arch/control.hpp>
#include <atomic>

namespace std {

class spinlock_mutex_irq {
private:
  std::atomic_flag m_locked = ATOMIC_FLAG_INIT;

  spinlock_mutex_irq(const spinlock_mutex_irq &) = delete;
  spinlock_mutex_irq &operator=(const spinlock_mutex_irq &) = delete;

public:
  spinlock_mutex_irq() = default;

  std::uint64_t lock() noexcept {
    std::uint64_t rflags = arch::irq::read_rflags();

    while (m_locked.test_and_set(std::memory_order_acquire)) {
      while (m_locked.test(std::memory_order_relaxed))
        arch::pause();
    }

    return rflags;
  }

  void unlock(std::uint64_t rflags) noexcept {
    m_locked.clear(std::memory_order_release);

    if (rflags & (1 << 9))
      arch::irq_enable();
  }

  std::optional<std::uint64_t> try_lock() noexcept {
    std::uint64_t rflags = arch::irq::read_rflags();

    if (m_locked.test_and_set(std::memory_order_acquire))
      return std::nullopt;

    return rflags;
  }
};

} // namespace std
