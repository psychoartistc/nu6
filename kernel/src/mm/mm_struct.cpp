#include <mm/mm_struct.hpp>

namespace mm {
kernel_vma_tree_t &get_kernel_vma_tree() {
  static kernel_vma_tree_t tree{KERNEL_VMA_START, KERNEL_VMA_END};
  return tree;
}

static page_table &kpt_get_impl() {
  static page_table a{};
  a.clone_bootloader_pages();
  a.switch_to();

  return a;
}

page_table &get_kernel_pt() {
  static auto &pt = kpt_get_impl();
  return pt;
}

} // namespace mm
