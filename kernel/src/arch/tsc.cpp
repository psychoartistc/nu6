#include <arch/tsc.hpp>

namespace arch::tsc {
std::uint64_t read() {
  std::uint32_t lo, hi;
  asm volatile("lfence; rdtsc" : "=a"(lo), "=d"(hi)::"memory");

  return (static_cast<std::uint64_t>(hi) << 32) | lo;
}

static inline bool inv_supported_impl() {
  std::uint32_t eax, ebx, ecx, edx;
  cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
  if (eax < 0x80000007)
    return false;

  cpuid(0x80000007, &eax, &ebx, &ecx, &edx);

  return (edx & (1 << 8)) != 0;
}

bool invariant_supported() {
  static bool cache = inv_supported_impl();
  return cache;
}

static inline std::uint64_t hz_impl() {
  std::uint32_t eax, ebx, ecx, edx;
  cpuid(&eax, &ebx, &ecx, &edx);

  if (eax < 0x15)
    return 0;

  cpuid(0x15, &eax, &ebx, &ecx, &edx);
  if (!eax || !ebx || !ecx)
    return 0;

  return static_cast<std::uint64_t>(ecx) * ebx / eax;
}

std::uint64_t herz_from_cpuid() {
  static std::uint64_t cache = hz_impl();
  return cache;
}
} // namespace arch::tsc
