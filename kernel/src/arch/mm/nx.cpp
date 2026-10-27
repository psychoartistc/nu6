#pragma once

#include <arch/cpuid.hpp>
#include <arch/mm/nx.hpp>

namespace arch::mm {
static bool nx_bit_supported_impl() {
  std::uint32_t eax, ebx, ecx, edx;

  cpuid(CPUID_EXT_FUNCTION_LEAF, &eax, &ebx, &ecx, &edx);
  if (eax < CPUID_EXT_FEATURES_LEAF)
    return false;

  cpuid(CPUID_EXT_FEATURES_LEAF, &eax, &ebx, &ecx, &edx);

  return (edx & (1 << 11)) != 0;
}

bool nx_bit_supported() {
  static bool cache = nx_bit_supported_impl();
  return cache;
}

void nx_bit_enable() {
  uint64_t efer = io::rdmsr(IA32_EFER);
  efer |= IA32_EFER_NXE;

  io::wrmsr(IA32_EFER, efer);
}
} // namespace arch::mm
