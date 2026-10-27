#include <arch/mm/paging.hpp>
#include <arch/cpuid.hpp>

namespace arch::mm {
static bool megapages_supported_impl() {
  uint32_t eax, ebx, ecx, edx;

  cpuid(CPUID_EXT_FUNCTION_LEAF, &eax, &ebx, &ecx, &edx);
  if (eax < CPUID_EXT_FEATURES_LEAF)
    return false;

  cpuid(CPUID_EXT_FEATURES_LEAF, &eax, &ebx, &ecx, &edx);

  return (edx & (1 << 26)) != 0;
}

bool megapages_supported() {
  static bool cache = megapages_supported_impl();

  return cache;
}
} // namespace arch::mm
