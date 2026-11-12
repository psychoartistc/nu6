#pragma once

#include <logging/logging.hpp>
#include <utils/mmio.hpp>
#include <cstddef>
#include <cstdint>

namespace hardware::ctl {
class lapic {
private:
  static inline constexpr std::uint64_t MSR_IA32_APIC_BASE = 0x01B;
  static inline constexpr std::uint64_t MSR_IA32_TSC_DEADLINE = 0x6E0;
  static inline constexpr std::uint64_t MSR_X2APIC_BASE = 0x800;
  static inline constexpr std::uint64_t APIC_BASE_X2APIC_EN = (1ULL << 10);

private:
  static logging::named_logger s_logger;
  static std::uintptr_t s_physBase;
  static utils::mmio s_mmio;
  static bool s_isX2Apic;

public:
  static void initialize(std::uintptr_t base);

  static std::uintptr_t physical_base();
};
} // namespace hardware::ctl
