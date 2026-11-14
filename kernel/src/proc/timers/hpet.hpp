#pragma once

#include <expected>
#include <cstddef>
#include <cstdint>
#include <utils/mmio.hpp>

#define HPET_TIMER_CONFIG(n) (0x100 + 0x20 * (n))
#define HPET_TIMER_COMPARE(n) (0x108 + 0x20 * (n))

namespace proc::timers {
class hpet {
private:
  enum class error_code { overflow, deadline_too_large, not_calibrated };

private:
  static inline constexpr std::uint64_t CAP = 0x000;
  static inline constexpr std::uint64_t GEN_CONFIG = 0x010;
  static inline constexpr std::uint64_t INT_STATUS = 0x020;
  static inline constexpr std::uint64_t MAIN_COUNTER = 0x0F0;
  static inline constexpr std::uint64_t LEG_RT_CAP = (1ULL << 15);

  static inline constexpr std::uint64_t TIMER_INDEX = 0;

  static inline constexpr std::uint64_t ENABLE = (1ULL << 0);
  static inline constexpr std::uint64_t LEGACY_REPLACE = (1ULL << 1);

  static inline constexpr std::uint64_t COUNTER_64 = (1ULL << 13);

  static inline constexpr std::uint64_t TN_INT_TYPE = (1ULL << 1);
  static inline constexpr std::uint64_t TN_INT_ENABLE = (1ULL << 2);
  static inline constexpr std::uint64_t TN_PERIODIC = (1ULL << 3);
  static inline constexpr std::uint64_t TN_SIZE_64 = (1ULL << 5);
  static inline constexpr std::uint64_t TN_VALUE_SET = (1ULL << 6);
  static inline constexpr std::uint64_t TN_MODE_32 = (1ULL << 8);
  static inline constexpr std::uint64_t TN_ROUTE_SHIFT = 9;
  static inline constexpr std::uint64_t TN_ROUTE_MASK =
      (0x1FULL << TN_ROUTE_SHIFT);
  static inline constexpr std::uint64_t TN_FSB_ENABLE = (1ULL << 14);

  static inline constexpr std::uint64_t MIN_LEAD_TICKS = 32ULL;
  static inline constexpr std::uint64_t IRQ_ROUTE = 2;
  static inline constexpr std::uint64_t IRQ_VECTOR = 52;

private:
  utils::mmio m_mmio;
  std::uint64_t m_periodFs = 0;
  std::uint32_t m_nrTimers = 0;

  bool m_bCounter64 = false;
  bool m_bUse64Compare = false;
  bool m_bReady = false;
  bool m_bStarted = false;

private:
  std::uint64_t read_counter();
  std::expected<std::uint64_t, error_code> ns_to_ticks(std::uint64_t ns);

public:
  void arm_ns(std::uint64_t ns);
};
} // namespace proc::timers
