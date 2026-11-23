#include <proc/run_queue.hpp>
#include <proc/timer_source.hpp>
#include <proc/cfs.hpp>

namespace proc {
task *run_queue::largest_vruntime() {
  if (m_tree.empty())
    return nullptr;

  return const_cast<task *>(std::addressof(*m_tree.rbegin()));
}

task *run_queue::least_vruntime() {
  if (m_tree.empty())
    return nullptr;

  return const_cast<task *>(std::addressof(*m_tree.begin()));
}

void run_queue::update_min_vruntime() {
  bool have = false;
  std::uint64_t candidate = 0;

  task *c = current();
  if (c && c->has_flag(task::flags::idle) &&
      c->get_state() == task::flags::running) {
    candidate = c->vruntime();
    have = true;
  }

  task *l = least_vruntime();
  if (l) {
    candidate = have ? std::min(candidate, l->vruntime()) : l->vruntime();
    have = true;
  }

  if (have)
    m_minVruntime = std::max(min_vruntime(), candidate);
}

void run_queue::update_current_vdelta() {
  task *c = current();
  timer_source *t = timer_source::current();
  if (!c || !t)
    return;

  std::uint64_t now = t->now_ns();

  // shouldnt overflow
  std::uint64_t delta = now - c->m_execStart;

  c->m_execStart = delta;
  if (!c->has_flag(task::flags::idle)) {
    c->m_sumExec += delta;
    c->m_vruntime += cfs::calculate_vdelta(delta, c->m_weight);
  }

  update_min_vruntime();
}

} // namespace proc
