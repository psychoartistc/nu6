#pragma once

#include <proc/timers/hpet.hpp>
#include <proc/timer_source.hpp>

namespace proc::timers {
class hpet_source : public timer_source {
private:
  hpet m_hpet;
  logging::named_logger m_logger{"hpet_source"};
  std::function<void()> m_fallbackHandler;

private:
  inline void fallback() {
    if (m_fallbackHandler)
      m_fallbackHandler();
  }

public:
  inline constexpr std::uint8_t type() const override {
    return timer_source::TYPE_HPET;
  }

  inline constexpr std::string_view name() const override { return "hpet"; }
  inline constexpr bool is_local() const override { return false; }

  std::uint64_t now_ns() override;

  bool is_available() override;

  void start(std::uint64_t initialDeadline) override;

  void next_deadline_set(std::uint64_t deadline) override;

  void add_fallback_handler(std::function<void()> fn) override;
};
}; // namespace proc::timers
