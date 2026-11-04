#pragma once

#include <mm/virtual/vma_tree.hpp>
#include <mm/slub.hpp>
#include <mm/virtual/page_table.hpp>
#include <mm/physical/slub_backend.hpp>

namespace mm {
std::expected<void, page_table::error_code>
map_vma(page_table &pt, vma &vma, protection prot, bool bypass_wxorx = false);

template <typename T = std::allocator<vma>>
std::expected<void *, page_table::error_code>
allocate_virtual(vma_tree<T> &tree, page_table &pt, std::size_t size,
                 protection prot, bool no_demand_paging = false,
                 bool bypass_wxorx = false) {
  if (!bypass_wxorx && prot_violates_wxorx(prot))
    return std::unexpected<page_table::error_code>(
        page_table::error_code::wxorx_violation);

  auto res = tree.allocate(size);
  if (!res)
    return std::unexpected<page_table::error_code>(
        page_table::error_code::tree_error);

  if (no_demand_paging) {
    // map immediately
    auto res2 = map_vma(pt, *res, prot, bypass_wxorx);
    if (!res2)
      return std::unexpected<page_table::error_code>(res2.error());
  }

  res->bypass_wxorx = bypass_wxorx;
  res->protect = prot;

  // in demand paging, mapping and physical allocation
  // is done when a page fault occurs, so we ignore mapping here obviously
  // and return an unmapped address
  return reinterpret_cast<void *>(res->address);
}

template <typename T>
std::expected<void, page_table::error_code>
free_virtual(vma_tree<T> &tree, page_table &pt, void *ptr) {
  vma *area = tree.find(reinterpret_cast<std::uintptr_t>(ptr));
  if (!area)
    return std::unexpected<page_table::error_code>(
        page_table::error_code::tree_error);

  if (area->mapped) {
    for (std::size_t i = 0; i < area->count; i++) {
      std::uintptr_t virt = area->address + i * PAGE_SIZE;
      auto res = pt.physical_of(virt);
      if (!res)
        return std::unexpected<page_table::error_code>(res.error());

      if (!page_allocator::free(*res, 1))
        return std::unexpected<page_table::error_code>(
            page_table::error_code::bad_address);
    }

    auto res = pt.unmap(area->address, area->count);
    if (!res)
      return std::unexpected<page_table::error_code>(res.error());
  }

  area->mapped = false;
  bool a = tree.free({area->address, area->count});
  if (!a) {
    pr_warn("failed to free vma 0x%p -> 0x%p back to tree", area->address,
            area->address + area->count * PAGE_SIZE);
    return std::unexpected<page_table::error_code>(
        page_table::error_code::tree_error);
  }

  return {};
}

} // namespace mm
