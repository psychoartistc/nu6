#pragma once

#include <arch/cpuid.hpp>
#include <cstdint>

namespace arch {
namespace irq {
static inline constexpr std::size_t MAX_IRQS = 256;
static inline constexpr std::uint64_t read_rflags() {
  std::uint64_t rflags;
  asm volatile("pushfq\n\t"
               "pop %0\n\t"
               : "=r"(rflags)
               :
               : "memory");

  return rflags;
}

static inline std::uint8_t read_boot_apic() {
  std::uint32_t eax = 1, ebx, ecx, edx;
  cpuid(&eax, &ebx, &ecx, &edx);

  return static_cast<std::uint8_t>(ebx >> 24);
}
} // namespace irq
} // namespace arch
