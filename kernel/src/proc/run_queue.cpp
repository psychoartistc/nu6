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

void run_queue::set_min_vruntime(std::uint64_t v) { m_minVruntime = v; }
} // namespace proc
