#pragma once

#include <atomic>
#include <arch/control.hpp>

namespace std {
template <typename EnumValue>
  requires std::is_enum_v<EnumValue>
class turnstile {
private:
  std::atomic<EnumValue> m_current{};

public:
  turnstile(EnumValue deflt) : m_current(deflt) {}

  void wait_for(EnumValue val) {}
  void set(EnumValue val) {}
};
} // namespace std
