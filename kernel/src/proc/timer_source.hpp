#pragma once

#include <logging/logging.hpp>
#include <std/vendor/veque.hpp>
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <bitset>

namespace proc {
class timer_source {
private:
  static inline constexpr std::uint8_t TIMER_SOURCE_LAPIC_TSC = 0;
  static inline constexpr std::uint8_t TIMER_SOURCE_HPET = 1;

private:
  static std::vector<std::unique_ptr<timer_source>> s_sources;
  static timer_source *s_pCurrent;
  static logging::named_logger s_logger;

private:
  static void initialize_sources();

  inline constexpr virtual std::uint8_t type() const = 0;
  inline constexpr virtual std::string_view name() const = 0;
  inline constexpr virtual bool is_local() const = 0;

  virtual ~timer_source();

  virtual std::uint64_t now_ns() = 0;

  virtual bool is_available() = 0;

  virtual void start(std::uint64_t initialDeadline) = 0;

  virtual void next_deadline_set(std::uint64_t deadline) = 0;
};
} // namespace proc
