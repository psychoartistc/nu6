#pragma once

#include <cstddef>
#include <cstdint>

namespace arch {
namespace mm {
static inline void invlpg(std::uintptr_t v) {
  asm volatile("invlpg (%0)" ::"r"(v) : "memory");
}
} // namespace mm
} // namespace arch
