#include <proc/timers/ltsc.hpp>
#include <modules/api.hpp>
#include <arch/io/msr.hpp>
#include <hardware/controllers/lapic.hpp>
#include <arch/irq/irq_dispatcher.hpp>
#include <proc/timers/pit.hpp>
#include <std/stubs/uint128.hpp>
#include <arch/tsc.hpp>

namespace proc::timers {
using namespace hardware::ctl;

static bool tsc_deadl_impl() {
  std::uint32_t eax, ebx, ecx, edx;

  arch::cpuid(1, &eax, &ebx, &ecx, &edx);

  return (ecx & (1 << 24)) != 0;
}

bool lapic_tsc::tsc_deadline_supported() {
  static bool cache = tsc_deadl_impl();
  return cache;
}

std::uint64_t lapic_tsc::now_ns() {
  std::uint64_t base = m_tscBase.load(std::memory_order_acquire);
  std::uint64_t now = arch::tsc::read();

  if (now < base) {
    m_logger.warn("now (%llu) < base (%llu), falling back", now, base);
    fallback();
    return 0;
  }

  std::uint64_t elapsed = now - base;

  std::uint128_t ns = static_cast<std::uint128_t>(elapsed) * 1000000000ULL;
  return static_cast<std::uint64_t>(ns / m_tscHz);
}

bool lapic_tsc::is_available() {
  return arch::tsc::invariant_supported() && tsc_deadline_supported();
}

void lapic_tsc::next_deadline_set(std::uint64_t deadline) {
  std::uint128_t ticks =
      (static_cast<std::uint128_t>(deadline) * m_tscHz + 999999999ULL) /
      1000000000ULL;
  if (ticks < 1)
    ticks = 1;

  std::uint64_t now = arch::tsc::read();

  if (ticks > static_cast<std::uint128_t>(~0ULL - now))
    ticks = ~0ULL - now;

  arch::io::wrmsr(lapic::MSR_IA32_TSC_DEADLINE,
                  now + static_cast<std::uint64_t>(ticks));
  m_logger.info("armed to %zu", deadline);
}

void lapic_tsc::start(std::uint64_t initialDeadline) {
  std::string_view hzMethod = "cpuid";
  std::uint64_t hz = arch::tsc::herz_from_cpuid();
  if (!hz) {
    hz = pit::tsc_herz();
    hzMethod = "pit";
  }

  if (!hz) {
    m_logger.warn("couldnt determine tsc frequency, falling back");
    fallback();
    return;
  }

  m_tscHz = hz;

  std::uint32_t svr = lapic::read(lapic::SVR);

  if (!(svr & lapic::SVR_ENABLE)) {
    if ((svr & 0xFF) < 0x10)
      svr = (svr & ~0xFFU) | lapic::SPURIOUS_VECTOR;

    lapic::write(lapic::SVR, svr | lapic::SVR_ENABLE);
  }

  lapic::write(lapic::LVT_TIMER, lapic::LVT_TSC_DEADLINE | IRQ_VECTOR);
  asm volatile("mfence" ::: "memory");

  std::uint64_t expected = 0;
  std::uint64_t base = arch::tsc::read();

  m_tscBase.compare_exchange_strong(expected, base, std::memory_order_release,
                                    std::memory_order_relaxed);

  m_logger.info("started, %llu hz determined by \'%s\'", m_tscHz,
                hzMethod.data());

  arch::irq::irq_dispatcher::set(IRQ_VECTOR, timer_source::tick_handler);

  next_deadline_set(initialDeadline);
}

void lapic_tsc::add_fallback_handler(std::function<void()> fn) {
  m_fallbackHandler = fn;
}

static bool lapic_tsc_entry() {
  timer_source::add_source<lapic_tsc>();
  return true;
}

} // namespace proc::timers

REGISTER_MODULE(ltsc_timer, "lapic tsc timer",
                "starts and initializes the lapic timer in tsc deadline mode",
                "1.0.0", module::MDLLICENSE_DEFAULT,
                module::module_init_order::timer_source_populate,
                proc::timers::lapic_tsc_entry, NULL);
