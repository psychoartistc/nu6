#pragma once

#include <cstdint>

namespace arch {
namespace irq {
static inline constexpr std::uint64_t read_rflags() {
  std::uint64_t rflags;
  asm volatile("pushfq\n\t"
               "pop %0\n\t"
               : "=r"(rflags)
               :
               : "memory");
  return rflags;
}
} // namespace irq
} // namespace arch
