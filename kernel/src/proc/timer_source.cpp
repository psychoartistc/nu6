#include <proc/timer_source.hpp>
#include <limits>

namespace proc {
void timer_source::initialize_sources() {
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
  s_logger.info("using timer source %s", s_pCurrent->name().data());
}

std::vector<std::unique_ptr<timer_source>> timer_source::s_sources{};
timer_source *timer_source::s_pCurrent{nullptr};
logging::named_logger s_logger{"timer_source"};
} // namespace proc
