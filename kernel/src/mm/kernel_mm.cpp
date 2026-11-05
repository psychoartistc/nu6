#include <mm/kernel_mm.hpp>
#include <mm/physical/page_allocator.hpp>
#include <mm/slub.hpp>
#include <mm/virtual/aslr.hpp>

namespace mm {
static std::pair<std::uintptr_t, std::uintptr_t> get_kernel_aslr() {
  static auto bounds = aslr_manager::generate_kernel();
  return bounds;
}

kernel_vma_tree_t &get_kernel_vma_tree() {
  static kernel_vma_tree_t tree{get_kernel_aslr().first,
                                get_kernel_aslr().second};
  return tree;
}

static page_table &kpt_get_impl() {
  static page_table a{};
  a.clone_bootloader_pages();
  a.switch_to();

  page_allocator::reclaim_bootloader();

  return a;
}

page_table &get_kernel_pt() {
  static auto &pt = kpt_get_impl();
  return pt;
}

} // namespace mm
