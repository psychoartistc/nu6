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

std::uint64_t run_queue::schedule_slice(const task *t) const {
  std::uint32_t nr = m_nrRunning.load(std::memory_order_relaxed);
  if (nr == 0)
    nr = 1;

  std::uint64_t period = cfs::LATENCY_NS;
  if (nr > cfs::LATENCY_NS / cfs::MIN_GRAN_NS)
    period = static_cast<std::uint64_t>(nr) * cfs::MIN_GRAN_NS;

  // use the load weight if specified
  std::uint64_t load = m_loadWeight ? m_loadWeight : t->m_weight;
  std::uint64_t slice = period * t->m_weight / load;

  // account for min gran ns
  return slice < cfs::MIN_GRAN_NS ? cfs::MIN_GRAN_NS : slice;
}

void run_queue::account_in(task *t) {
  m_nrRunning.fetch_add(1, std::memory_order_relaxed);
  m_loadWeight += t->m_weight;
}

void run_queue::account_out(task *t) {
  m_nrRunning.fetch_sub(1, std::memory_order_relaxed);
  m_loadWeight -= t->m_weight;
}

void run_queue::enqueue_task(task *t) {
  assert(!t->m_bQueued && "attempted to queue an already queued task");
  m_tree.insert(t);
  t->m_bQueued = true;
}

void run_queue::dequeue_task(task *t) {
  assert(t->m_bQueued && "attempted to dequeue an already dequeued task");
  m_tree.erase(t);
  t->m_bQueued = false;
}

} // namespace proc
