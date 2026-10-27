#pragma once

#include <cstddef>
#include <cstdint>

namespace arch {
namespace io {
static inline std::uint64_t rdmsr(std::uint32_t msr) {
  std::uint32_t lo, hi;
  asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
  return ((std::uint64_t)hi << 32) | lo;
} // namespace std::uint64_t

static inline void wrmsr(std::uint32_t index, std::uint64_t value) {
  std::uint32_t eax = (std::uint32_t)(value & 0xFFFFFFFF);
  std::uint32_t edx = (std::uint32_t)(value >> 32);
  asm volatile("wrmsr" : : "c"(index), "a"(eax), "d"(edx));
}
} // namespace io
} // namespace arch
