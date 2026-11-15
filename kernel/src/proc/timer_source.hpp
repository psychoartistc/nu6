#pragma once

#include <memory>
#include <std/stubs/make_unique.hpp>
#include <logging/logging.hpp>
#include <std/vendor/veque.hpp>
#include <string_view>
#include <arch/tables/idt.hpp>
#include <cstddef>
#include <cstdint>
#include <bitset>

namespace proc {
class timer_source {
public:
  static inline constexpr std::uint8_t TYPE_LAPIC_TSC = 0;
  static inline constexpr std::uint8_t TYPE_HPET = 1;

private:
  static std::vector<std::unique_ptr<timer_source>> s_sources;
  static timer_source *s_pCurrent;
  static logging::named_logger s_logger;

public:
  static void initialize_sources();

  // should get called on a tick
  static void tick_handler(arch::interrupt_frame *frame);

  static timer_source *current();

  inline constexpr virtual std::uint8_t type() const = 0;
  inline constexpr virtual std::string_view name() const = 0;
  inline constexpr virtual bool is_local() const = 0;

  virtual ~timer_source() {}

  virtual std::uint64_t now_ns() = 0;

  virtual bool is_available() = 0;

  virtual void start(std::uint64_t initialDeadline) = 0;

  virtual void next_deadline_set(std::uint64_t deadline) = 0;

  // the callback should be stored byu derived classes
  // and then called once an error occurs
  // this switches the timer sources immediately and picks a new one
  virtual void add_fallback_handler(std::function<void()> fn) = 0;

  template <std::derived_from<timer_source> T> static void add_source() {
    std::unique_ptr<T> ty = std::make_unique<T>();
    s_sources.push_back(std::move(ty));
  }
};
} // namespace proc
