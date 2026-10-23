#pragma once

#include <logging/logging.hpp>
#include <std/spinlock.hpp>

namespace module {
class com1_serial {
private:
  static inline constexpr std::uint64_t COM1 = 0x3F8;

  static inline constexpr std::uint64_t COM1_DATA = (COM1 + 0);
  static inline constexpr std::uint64_t COM1_INTERRUPT = (COM1 + 1);
  static inline constexpr std::uint64_t COM1_BAUD_LOW = (COM1 + 0);
  static inline constexpr std::uint64_t COM1_BAUD_HIGH = (COM1 + 1);
  static inline constexpr std::uint64_t COM1_FIFO_CTRL = (COM1 + 2);
  static inline constexpr std::uint64_t COM1_LINE_CTRL = (COM1 + 3);
  static inline constexpr std::uint64_t COM1_MODEM_CTRL = (COM1 + 4);
  static inline constexpr std::uint64_t COM1_LINE_STATUS = (COM1 + 5);

private:
  logging::named_logger m_logger{"com1_serial"};
  bool m_bIsError = true;

public:
  bool is_transmit_empty();

  void put(char c);
  void puts(std::string_view c);

  void initialize();

  bool is_error() const { return m_bIsError; }

  void vputsf(std::string_view s, std::va_list va);
  void putsf(std::string_view s, ...);
};
} // namespace module
