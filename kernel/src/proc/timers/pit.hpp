#pragma once

#include <cstdint>

namespace proc::timers {

class pit {
private:
  static inline constexpr std::uint64_t HERZ = 1193182ULL;
  static inline constexpr std::uint64_t CAL_COUNT = 11932U;

public:
  static void disable();
  static std::uint64_t tsc_herz();
};
} // namespace proc::timers
