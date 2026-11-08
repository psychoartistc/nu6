#pragma once

#include <cstddef>
#include <cstdint>

namespace hardware::ctl {
class lapic {
private:
  std::uintptr_t m_physBase{0x0};

public:
  lapic(std::uintptr_t base);

  std::uintptr_t physical_base() const { return m_physBase; }
};
} // namespace hardware::ctl
