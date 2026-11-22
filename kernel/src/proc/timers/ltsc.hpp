#pragma once

#include <proc/timer_source.hpp>

namespace proc::timers {
class lapic_tsc : public timer_source {
private:
  static inline constexpr std::uint8_t IRQ_VECTOR = 33;

  std::function<void()> m_fallbackHandler;

private:
  logging::named_logger m_logger{"lapic_tsc"};
  std::atomic<std::uint64_t> m_tscBase{0x0};
  std::uint64_t m_tscHz{0x0};

private:
  bool tsc_deadline_supported();

  inline void fallback() {
    if (m_fallbackHandler)
      m_fallbackHandler();
  }

public:
  inline constexpr std::uint8_t type() const override {
    return timer_source::TYPE_LAPIC_TSC;
  }

  inline constexpr std::string_view name() const override { return "ltsc"; }
  inline constexpr bool is_local() const override { return true; }

  std::uint64_t now_ns() override;

  bool is_available() override;

  void start(std::uint64_t initialDeadline) override;

  void next_deadline_set(std::uint64_t deadline) override;

  void add_fallback_handler(std::function<void()> fn) override;
};
} // namespace proc::timers
