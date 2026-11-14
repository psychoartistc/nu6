#include <proc/timers/hpet.hpp>
#include <hardware/controllers/ioapic.hpp>
#include <mm/kernel_mm.hpp>
#include <proc/timers/pit.hpp>
#include <utils/math.hpp>
#include <std/stubs/uint128.hpp>
#include <arch/irq/irq_dispatcher.hpp>

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

std::expected<void, hpet::error_code>
hpet::initialize(std::function<void(arch::interrupt_frame *)> fn) {
  pit::disable();
  if (!s_base) {
    m_logger.warn("no hpet physical address");
    return std::unexpected<error_code>(error_code::no_physical_base);
  }

  auto &tree = mm::get_kernel_vma_tree();
  auto &pt = mm::get_kernel_pt();

  auto res = tree.allocate(4096);
  if (!res)
    return std::unexpected<error_code>(error_code::no_virtual_mem);

  auto res2 = pt.map(s_base, res->address, MMIO_PROT);
  if (!res2)
    return std::unexpected<error_code>(error_code::map_error);

  m_mmio.change_address(res->address);

  std::uint64_t cap = m_mmio.read<std::uint64_t>(CAP);

  m_periodFs = (cap >> 32) & 0xFFFFFFFFULL;
  m_nrTimers = static_cast<std::uint32_t>((cap >> 8) & 0x1FULL) + 1;

  if (!m_periodFs) {
    m_logger.warn("hardware gave an invalid counter period");
    return std::unexpected<error_code>(error_code::invalid_counter);
  }

  if (TIMER_INDEX >= m_nrTimers) {
    m_logger.warn("timer %zu looks unimplemented", TIMER_INDEX);
    return std::unexpected<error_code>(error_code::timer_unimplemented);
  }

  std::uint32_t tcfg_offset = HPET_TIMER_CONFIG(TIMER_INDEX);
  std::uint64_t tcfg = m_mmio.read<std::uint64_t>(tcfg_offset);

  std::uint32_t routeCaps = static_cast<std::uint32_t>(tcfg >> 32);
  if (!(routeCaps & (1U << IRQ_ROUTE))) {
    m_logger.warn("irq route %zu is not supported by timer 0", IRQ_ROUTE);
    return std::unexpected<error_code>(error_code::route_not_supported);
  }

  bool c64 = (cap & COUNTER_64) != 0;
  bool t64 = (cap & TN_SIZE_64) != 0;

  m_bUse64Compare = c64 && t64;
  m_bCounter64 = c64;

  tcfg &= ~(TN_INT_TYPE | TN_INT_ENABLE | TN_PERIODIC | TN_VALUE_SET |
            TN_MODE_32 | TN_ROUTE_MASK | TN_FSB_ENABLE);

  tcfg |= static_cast<std::uint64_t>(IRQ_ROUTE) << TN_ROUTE_SHIFT;

  if (!m_bUse64Compare)
    tcfg |= TN_MODE_32;

  m_mmio.write<std::uint64_t>(tcfg_offset, tcfg);

  arch::irq::irq_dispatcher::set(static_cast<std::uint8_t>(IRQ_VECTOR), fn);
  hardware::ctl::ioapic::all()[0].
}

void hpet::set_physical_base(std::uintptr_t base) { s_base = base; }

std::uintptr_t hpet::s_base{0x0};
} // namespace proc::timers
