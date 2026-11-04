#pragma once

#include <mm/slub.hpp>
#include <mm/virtual/page_table.hpp>
#include <mm/virtual/vma_tree.hpp>
#include <mm/physical/slub_backend.hpp>

namespace mm {
using kernel_vma_allocator_t =
    slub_stl_allocator<vma, PAGE_SIZE, page_slub_backend, false>;

using kernel_vma_tree_t = vma_tree<kernel_vma_allocator_t>;

kernel_vma_tree_t &get_kernel_vma_tree();
page_table &get_kernel_pt();

} // namespace mm
