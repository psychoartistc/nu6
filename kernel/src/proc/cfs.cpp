#include <proc/cfs.hpp>

namespace proc {
void cfs::update_min_vruntime(run_queue &rq) {
  bool have = false;
  std::uint64_t candidate = 0;

  task *c = rq.current();
  if (c && c->has_flag(task::flags::idle) &&
      c->get_state() == task::flags::running) {
    candidate = c->vruntime();
    have = true;
  }

  task *l = rq.least_vruntime();
  if (l) {
    candidate = have ? std::min(candidate, l->vruntime()) : l->vruntime();
    have = true;
  }

  if (have)
    rq.set_min_vruntime(std::max(rq.min_vruntime(), candidate));
}
} // namespace proc
