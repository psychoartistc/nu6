#pragma once

#include <mm/virtual/page_table.hpp>
#include <mm/virtual/vma_tree.hpp>
#include <mm/virtual/vma_defs.hpp>

namespace mm {
using kernel_vma_tree_t = vma_tree<pool_allocator<vma>>;

kernel_vma_tree_t &get_kernel_vma_tree();
page_table &get_kernel_pt();

template <typename T = std::allocator<vma>>
std::expected<void *, page_table::error_code>
allocate_virtual(vma_tree<T> &tree, page_table &pt, std::size_t size,
                 protection prot, bool no_demand_paging = false,
                 bool bypass_wxorx = false) {
  auto res = tree.allocate(size);
  if (!res)
    return std::unexpected<page_table::error_code>(
        page_table::error_code::no_memory);

  if (no_demand_paging) {
    // map immediately
    // TODO: greedy distribution algorithm here to map
    // as many 1gb pages and 2mb pages possible
    for (std::size_t i = 0; i < res->count; i++) {
      auto res2 = page_allocator::allocate(1);
      if (!res2)
        return std::unexpected<page_table::error_code>(
            page_table::error_code::no_memory);

      auto res3 = pt.map(*res2, res->address + i * PAGE_SIZE, prot,
                         page_table::page_type::normal, bypass_wxorx);
      if (!res3)
        return std::unexpected<page_table::error_code>(res3.error());
    }
  } else if (!bypass_wxorx && prot_violates_wxorx(prot))
    return std::unexpected<page_table::error_code>(
        page_table::error_code::wxorx_violation);

  // in demand paging, mapping and physical allocation
  // is done when a page fault occurs, ignore it here obviously

  return reinterpret_cast<void *>(res->address);
}
} // namespace mm
