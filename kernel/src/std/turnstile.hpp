#pragma once

#include <atomic>
#include <type_traits>
#include <arch/control.hpp>

namespace std {

template <typename EnumValue>
  requires std::is_enum_v<EnumValue>
class turnstile {
private:
  std::atomic<EnumValue> m_current{};

public:
  explicit turnstile(EnumValue deflt) : m_current(deflt) {}

  void wait_for(EnumValue val) const noexcept {
    while (m_current.load(std::memory_order_acquire) < val) {
      arch::pause();
    }
  }

  void set(EnumValue val) noexcept {
    m_current.store(val, std::memory_order_release);
  }

  EnumValue current() const noexcept {
    return m_current.load(std::memory_order_acquire);
  }
};

} // namespace std
