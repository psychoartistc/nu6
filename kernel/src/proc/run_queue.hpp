#pragma once

#include <proc/task.hpp>
#include <std/dsa/rbtree.hpp>

namespace proc {
struct task_less {
  using is_transparent = void;

  constexpr bool operator()(const task &a, const task &b) const noexcept {
    return a.vruntime() < b.vruntime();
  }
  constexpr bool operator()(const task &a, std::uint64_t p) const noexcept {
    return a.vruntime() < p;
  }
  constexpr bool operator()(std::uint64_t p, const task &a) const noexcept {
    return p < a.vruntime();
  }
};

class run_queue {
private:
  std::rb_tree<task, std::rb_identity, task_less> m_tree;

  task *m_current{nullptr}, *m_idle{nullptr}, *m_zombie{nullptr},
      *m_sleepers{nullptr};

  std::uint64_t m_loadWeight{0};
  std::uint64_t m_minVruntime{0};
  std::uint64_t m_nrSwitches{0};
  std::uint64_t m_nrRunning{0};

  bool m_online{false};
  bool m_needResched{false};
};
} // namespace proc
