#pragma once

#include <mm/virtual/protection.hpp>
#include <concepts>
#include <cstddef>
#include <cstdint>

namespace utils {
class mmio {
public:
  static inline constexpr mm::protection PROT =
      mm::protection::kernel | mm::protection::read | mm::protection::write |
      mm::protection::nocache;

private:
  std::uint8_t *m_address = nullptr;

public:
  mmio(std::uintptr_t virt)
      : m_address(reinterpret_cast<std::uint8_t *>(virt)) {}

  template <std::integral T> T read(std::uint64_t offset) {
    return *reinterpret_cast<T *>(m_address + offset);
  }

  template <std::integral T> void write(std::uint64_t offset, T val) {
    *reinterpret_cast<T *>(m_address + offset) = val;
  }

  inline void change_address(std::uintptr_t a) {
    m_address = reinterpret_cast<std::uint8_t *>(a);
  }
};
} // namespace utils
