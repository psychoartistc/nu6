#pragma once

#include <cstdint>

namespace arch {
static inline constexpr std::uint64_t CPUID_EXT_FEATURES_LEAF = 0x80000001;
static inline constexpr std::uint64_t CPUID_EXT_FUNCTION_LEAF = 0x80000000;

static inline void cpuid(std::uint32_t leaf, std::uint32_t *eax,
                         std::uint32_t *ebx, std::uint32_t *ecx,
                         std::uint32_t *edx) {
  asm volatile("cpuid"
               : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
               : "a"(leaf));
}
} // namespace arch
