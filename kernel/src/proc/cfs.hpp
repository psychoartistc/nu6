#pragma once

#include <cstdint>

namespace proc {
class cfs {
public:
  static inline constexpr std::uint32_t PRIO_TO_WEIGHT[40] = {
      88761, 71755, 56483, 46273, 36291, 29154, 23254, 18705, 14949, 11916,
      9548,  7620,  6100,  4904,  3906,  3121,  2501,  1991,  1586,  1277,
      1024,  820,   655,   526,   423,   335,   272,   215,   172,   137,
      110,   87,    70,    56,    45,    36,    29,    23,    18,    15,
  };

  static inline constexpr std::uint64_t LATENCY_NS = 6000000ULL;
  static inline constexpr std::uint64_t MIN_GRAN_NS = 750000ULL;
  static inline constexpr std::uint64_t WAKEUP_GRAN_NS = 1000000ULL;
  static inline constexpr std::uint64_t IDLE_TICK_NS = 1000000ULL;
  static inline constexpr std::uint64_t MIN_TICK_NS = 50000ULL;
  static inline constexpr std::uint64_t KICK_NS = 1000ULL;
  static inline constexpr std::uint64_t NICE0_WEIGHT = 1024;

public:
  // calculates vdelta from vruntime and the nice's weight
  static inline constexpr std::uint64_t calculate_vdelta(std::uint64_t delta_ns,
                                                         std::uint32_t weight) {
    if (weight == 0)
      return 0;

    return (delta_ns * NICE0_WEIGHT) / weight;
  }
};
} // namespace proc
