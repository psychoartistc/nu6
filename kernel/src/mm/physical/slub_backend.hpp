#pragma once

#include <mm/common.hpp>
#include <mm/physical/page_allocator.hpp>

namespace mm {
// WARNING this is very terrible and shouldnt be used anywhere outside of
// the vma tree's slub backend
class page_slub_backend {
public:
  void *allocate() {
    auto res = page_allocator::allocate(1);
    if (!res)
      return nullptr;

    pr_info("allocated phys at 0x%p", p2v(*res));
    return p2v(*res);
  }

  void deallocate(void *ptr) { page_allocator::free(v2p(ptr), 1); }
};
} // namespace mm
