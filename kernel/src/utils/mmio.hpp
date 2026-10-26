#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace utils {
class mmio {
private:
  std::uint8_t *m_address = nullptr;

public:
  mmio(std::uintptr_t virt)
      : m_address(reinterpret_cast<std::uint8_t *>(virt)) {}

  template <std::integral T> void read(std::uint64_t offset) {
    return *reinterpret_cast<T *>(m_address + offset);
  }

  template <std::integral T> void write(std::uint64_t offset, T val) {
    *reinterpret_cast<T *>(m_address + offset) = val;
  }
};
} // namespace utils
