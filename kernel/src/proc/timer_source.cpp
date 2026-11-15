#include <proc/timer_source.hpp>
#include <hardware/controllers/lapic.hpp>
#include <modules/api.hpp>
#include <limits>

namespace proc {
void timer_source::initialize_sources() {
  module::initialize_some(module::module_init_order::timer_source_populate);

  std::uint8_t best = std::numeric_limits<std::uint8_t>::max();
  for (auto &p : s_sources) {
    if (!p)
      continue;

    bool available = p->is_available();
    if (!available)
      continue;

    if (p->type() < best) {
      s_pCurrent = p.get();
      best = p->type();
    }
  }

  assert(s_pCurrent && "no available timer source found, fatal");
  s_logger.info("using timer source \'%s\'", s_pCurrent->name().data());
}

void timer_source::tick_handler(arch::interrupt_frame *frame) {
  pr_info("hello! int no: %zu", frame->int_no);

  hardware::ctl::lapic::eoi();

  auto *curr = current();
  curr->next_deadline_set(1000000000);
}

timer_source *timer_source::current() { return s_pCurrent; }

std::vector<std::unique_ptr<timer_source>> timer_source::s_sources{};
timer_source *timer_source::s_pCurrent{nullptr};
logging::named_logger timer_source::s_logger{"timer_source"};
} // namespace proc
