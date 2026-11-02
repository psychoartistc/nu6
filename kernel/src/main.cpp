#include <std/memory.hpp>
#include <mm/mm_struct.hpp>
#include <mm/virtual/vma_tree.hpp>
#include <utils/math.hpp>
#include <limine.h>
#include <std/enums.hpp>
#include <mm/physical/page_allocator.hpp>
#include <arch/tables/gdt.hpp>
#include <logging/logging.hpp>
#include <modules/api.hpp>
#include <stdbool.h>
#include <stdint.h>
#include <stdnoreturn.h>
#include <std/spinlock.hpp>
#include <abi/itanium.hpp>
#include <arch/tables/tss.hpp>
#include <arch/tables/idt.hpp>
#include <mm/virtual/vma_defs.hpp>
#include <arch/tables/ist.hpp>
#include <std/dsa/rbtree.hpp>
#include <mm/virtual/page_table.hpp>
#include <mm/stl.hpp>

extern "C" void kmain() {
  cxxabi::call_global_ctors();
  module::initialize_some(module::module_init_order::urgent);
  arch::smp::core_manager::launch_all();

  bool r = mm::page_allocator::initialize();
  if (!r)
    utils::panic("failed to initialize page allocator");

  auto res = mm::page_allocator::allocate(1);
  if (!res)
    pr_error("err: %d", static_cast<int>(res.error()));
  else
    pr_info("0x%p", *res);

  auto &pt = mm::get_kernel_pt();
  auto &tree = mm::get_kernel_vma_tree();
  int *num = new int(4);
  pr_info("%d", *num);

  arch::hcf();
}
