#pragma once

#include <proc/task.hpp>
#include <std/dsa/rbtree.hpp>
#include <atomic>

namespace proc {
struct task_less {
  using is_transparent = void;

  constexpr bool operator()(const task *a, const task *b) const noexcept {
    if (a->vruntime() != b->vruntime()) {
      return a->vruntime() < b->vruntime();
    }

    return std::less<>()(std::addressof(a), std::addressof(b));
  }

  constexpr bool operator()(const task *a, std::uint64_t p) const noexcept {
    return a->vruntime() < p;
  }

  constexpr bool operator()(std::uint64_t p, const task *a) const noexcept {
    return p < a->vruntime();
  }
};

class run_queue {
private:
  std::rb_tree<task *, std::rb_identity, task_less> m_tree;

  task *m_current{nullptr}, *m_idle{nullptr}, *m_zombie{nullptr},
      *m_sleepers{nullptr};

  std::uint64_t m_loadWeight{0};
  std::uint64_t m_minVruntime{0};
  std::uint64_t m_nrSwitches{0};
  std::atomic<uint64_t> m_nrRunning{0};

  bool m_online{false};
  bool m_needResched{false};

public:
  task *largest_vruntime();
  task *least_vruntime();

  task *current() const { return m_current; }
  task *idle() const { return m_idle; }
  task *zombie() const { return m_zombie; }
  task *sleepers() const { return m_sleepers; }

  std::uint64_t min_vruntime() const { return m_minVruntime; }

  void update_min_vruntime();

  void update_current_vdelta();

  std::uint64_t schedule_slice(const task *t) const;

  void account_in(task *t);
  void account_out(task *t);

  void enqueue_task(task *t);
  void dequeue_task(task *t);
};
} // namespace proc
