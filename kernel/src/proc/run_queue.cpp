#include <proc/run_queue.hpp>

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
} // namespace proc
