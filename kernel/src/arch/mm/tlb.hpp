#pragma once

#include <cstddef>
#include <cstdint>

namespace arch {
namespace mm {
static inline void invlpg(std::uintptr_t v) {
  asm volatile("invlpg (%0)" ::"r"(v) : "memory");
}

static inline void flush_all() {
  std::uint64_t cr4;
  asm volatile("mov %%cr4, %0" : "=r"(cr4));
  asm volatile("mov %0, %%cr4" ::"r"(cr4 & ~(1ull << 7)) : "memory");
  asm volatile("mov %0, %%cr4" ::"r"(cr4) : "memory");
}
} // namespace mm
} // namespace arch
