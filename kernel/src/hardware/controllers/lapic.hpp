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

  static inline constexpr std::uint64_t LAPIC_ICR_SHORTHAND_ALL_BUT_SELF =
      (2 << 18);

  static inline constexpr std::uint64_t LAPIC_REG_ICR_LOW = 0x300;
  static inline constexpr std::uint64_t LAPIC_REG_ICR_HIGH = 0x310;

  static inline constexpr std::uint64_t LAPIC_ICR_DELIVERY_STATUS = (1 << 12);

  static inline constexpr std::uint64_t LAPIC_EOI = 0x0B0;

private:
  static std::uint32_t read(std::uint32_t reg);
  static void write(std::uint32_t reg, std::uint32_t value);

private:
  static logging::named_logger s_logger;
  static std::uintptr_t s_physBase;
  static utils::mmio s_mmio;
  static bool s_isX2Apic;

public:
  static void initialize(std::uintptr_t base);

  static std::uintptr_t physical_base();

  static void eoi();

  // sends an interrupt to every core but self
  static void ipi_except_self(std::uint8_t vec);
};
} // namespace hardware::ctl
