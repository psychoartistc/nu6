#pragma once

#include <logging/logging.hpp>
#include <cstddef>
#include <cstdint>
#include <arch/io/ports.hpp>

namespace hardware::ctl {
// whole class just to get disabled...
class pic8259 {
private:
  static logging::named_logger s_logger;

  static inline constexpr std::uint16_t PIC1_COMMAND = 0x20;
  static inline constexpr std::uint16_t PIC1_DATA = 0x21;
  static inline constexpr std::uint16_t PIC2_COMMAND = 0xA0;
  static inline constexpr std::uint16_t PIC2_DATA = 0xA5;

public:
  static void disable();
};
} // namespace hardware::ctl
