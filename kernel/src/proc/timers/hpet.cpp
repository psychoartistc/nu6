#include <proc/timers/hpet.hpp>
#include <utils/math.hpp>
#include <std/stubs/uint128.hpp>

namespace proc::timers {
std::uint64_t hpet::read_counter() {
  if (m_bCounter64)
    return m_mmio.read<std::uint64_t>(MAIN_COUNTER);

  return m_mmio.read<std::uint32_t>(MAIN_COUNTER);
}

std::expected<std::uint64_t, hpet::error_code>
hpet::ns_to_ticks(std::uint64_t ns) {
  std::uint128_t scaled = (std::uint128_t)ns * 1000000ULL;
  scaled = utils::div_ceil<std::uint128_t>(scaled, m_periodFs);

  if (scaled < 1)
    scaled = 1;

  scaled += MIN_LEAD_TICKS;

  if (m_bUse64Compare) {
    if (scaled > 0x7FFFFFFFFFFFFFFFULL)
      return std::unexpected<error_code>(error_code::overflow);
  } else {
    if (scaled > 0x7FFFFFFFULL)
      return std::unexpected<error_code>(error_code::overflow);
  }

  return static_cast<std::uint64_t>(scaled);
}

std::expected<void, hpet::error_code> hpet::arm_ns(std::uint64_t ns) {
  auto res = ns_to_ticks(ns);
  if (!res) {
    std::uint64_t cfg =
        m_mmio.read<std::uint64_t>(HPET_TIMER_CONFIG(TIMER_INDEX));

    m_mmio.write<std::uint64_t>(HPET_TIMER_CONFIG(TIMER_INDEX),
                                cfg & ~TN_INT_ENABLE);

    m_logger.warn("deadline %zu too large for the comparator mode", ns);
    return std::unexpected<error_code>(res.error());
  }

  std::uint32_t tcfg_offset = HPET_TIMER_CONFIG(TIMER_INDEX);
  std::uint64_t tcfg = m_mmio.read<std::uint64_t>(tcfg_offset);

  tcfg &= ~TN_INT_ENABLE;
  m_mmio.write<std::uint64_t>(tcfg_offset, tcfg);

  if (m_bUse64Compare) {
    std::uint64_t now = m_mmio.read<std::uint64_t>(MAIN_COUNTER);
    m_mmio.write<std::uint64_t>(HPET_TIMER_COMPARE(TIMER_INDEX), now + *res);
  } else {
    std::uint32_t now = m_mmio.read<std::uint32_t>(MAIN_COUNTER);
    std::uint32_t target = now + static_cast<std::uint32_t>(*res);

    m_mmio.write<std::uint32_t>(HPET_TIMER_COMPARE(TIMER_INDEX), target);
  }

  tcfg &= ~(TN_INT_TYPE | TN_PERIODIC | TN_VALUE_SET | TN_FSB_ENABLE |
            TN_INT_ENABLE);

  m_mmio.write<std::uint64_t>(tcfg_offset, tcfg | TN_INT_ENABLE);

  return {};
}

std::expected<void, hpet::error_code> hpet::start(std::uint64_t deadline) {
  std::uint64_t cfg = m_mmio.read<std::uint64_t>(GEN_CONFIG);

  cfg &= ~ENABLE;
  m_mmio.write<std::uint64_t>(GEN_CONFIG, cfg);

  if (m_bCounter64)
    m_mmio.write<std::uint64_t>(MAIN_COUNTER, 0);
  else
    m_mmio.write<std::uint32_t>(MAIN_COUNTER, 0);

  cfg |= ENABLE | LEGACY_REPLACE;
  m_mmio.write<std::uint64_t>(GEN_CONFIG, cfg);

  auto res = arm_ns(deadline);
  if (!res)
    return std::unexpected<error_code>(res.error());

  m_bStarted = true;

  m_logger.info("timer started with initial delay %llu ns", deadline);

  return {};
}

std::expected<void, hpet::error_code> hpet::initialize() {}

std::uintptr_t hpet::m_base{0x0};
} // namespace proc::timers
