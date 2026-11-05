#pragma once

#include <mm/slub.hpp>
#include <mm/virtual/allocator.hpp>
#include <mm/kernel_mm.hpp>

namespace mm {
static inline constexpr protection SLUB_SLAB_PROT =
    protection::read | protection::write | protection::kernel;

static inline constexpr bool SLUB_SLAB_DEMAND_PAGING = false;

static inline constexpr std::size_t SLUB_SLAB_SIZE = 16384;

class kern_vmem_slub_backend {
private:
  logging::named_logger m_logger{"vmem_slub_backend"};

public:
  void *allocate(std::size_t size) {
    auto res = allocate_virtual<kernel_vma_allocator_t>(
        get_kernel_vma_tree(), get_kernel_pt(), size, SLUB_SLAB_PROT,
        !SLUB_SLAB_DEMAND_PAGING, false);
    if (!res) {
      m_logger.warn("bad allocation! size: %zu, error: %d", size,
                    static_cast<int>(res.error()));
      return nullptr;
    }

    return *res;
  }

  void deallocate(void *p, std::size_t s) {
    (void)s;
    auto res = free_virtual<kernel_vma_allocator_t>(get_kernel_vma_tree(),
                                                    get_kernel_pt(), p);
    if (!res) {
      m_logger.warn("failed to free virtual! size: %zu, error: %d", s,
                    static_cast<int>(res.error()));
      return;
    }
  }
};

template <typename T>
using kernel_slub_allocator_t =
    slub_stl_allocator<T, PAGE_SIZE, kern_vmem_slub_backend, true>;

using slub_alloc_t =
    slub::slub_allocator<SLUB_SLAB_SIZE, kern_vmem_slub_backend, true>;

} // namespace mm
