#include <proc/timers/hpet.hpp>
#include <modules/api.hpp>
#include <vendor/magic_enum.hpp>
#include <proc/timers/hpet_source.hpp>

// TODO: logs here
namespace proc::timers {
std::uint64_t hpet_source::now_ns() { return m_hpet.now_ns(); }

bool hpet_source::is_available() {
  // by my (stupid) design even if the hpet is broken
  // or not available we fallback via the checks,
  // so this doesnt matter and we can always return true
  return true;
}

void hpet_source::start(std::uint64_t initialDeadline) {
  auto res = m_hpet.initialize(timer_source::tick_handler);

  if (!res) {
    m_logger.warn("initialize() failed, falling back: %s",
                  magic_enum::enum_name(res.error()));
    fallback();
    return;
  }

  auto res2 = m_hpet.start(initialDeadline);
  if (!res2) {
    m_logger.warn("start() failed, falling back: %s",
                  magic_enum::enum_name(res2.error()));
    fallback();
  }
}

void hpet_source::next_deadline_set(std::uint64_t deadline) {
  auto res = m_hpet.arm_ns(deadline);
  if (!res) {

    m_logger.warn("arm_ns() failed, falling back: %s",
                  magic_enum::enum_name(res.error()));
    fallback();
  }
}

void hpet_source::add_fallback_handler(std::function<void()> fn) {
  m_fallbackHandler = fn;
}

static bool hpet_source_entry() {
  timer_source::add_source<hpet_source>();
  return true;
}
} // namespace proc::timers

REGISTER_MODULE(hpet_timer, "hpet timer",
                "starts and initializes the hpet timer", "1.0.0",
                module::MDLLICENSE_DEFAULT,
                module::module_init_order::timer_source_populate,
                proc::timers::hpet_source_entry, NULL);
