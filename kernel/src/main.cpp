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
#include <mm/physical/freelist/freelist.hpp>
#include <mm/virtual/page_table.hpp>

extern "C" void kmain() {
  cxxabi::call_global_ctors();
  module::initialize_some(module::module_init_order::urgent);

  bool r = mm::page_allocator::initialize();
  if (!r)
    utils::panic("failed to initialize page allocator");

  auto res = mm::page_allocator::allocate(1);
  if (!res)
    pr_error("err: %d", static_cast<int>(res.error()));
  else
    pr_info("0x%p", *res);

  auto &tree = mm::get_kernel_vma_tree();
  auto &pt = mm::get_kernel_pt();

  auto res2 = mm::allocate_virtual(
      tree, pt, 16384,
      mm::protection::kernel | mm::protection::write | mm::protection::read,
      true);
  if (!res2)
    pr_info("allocate_virtual fail");

  int *addr = reinterpret_cast<int *>(*res2);
  *addr = 5;
  pr_info("%d", *addr);

  auto res3 = tree.find(reinterpret_cast<std::uintptr_t>(addr));
  if (!res3)
    pr_error("lookup fail");

  pr_info("address 0x%p belongs to vma 0x%p -> 0x%p, mapped: %d", addr,
          res3->address, res3->address + res3->count * mm::PAGE_SIZE,
          res3->mapped);

  auto res4 = mm::free_virtual(tree, pt, addr);
  if (!res4)
    pr_warn("fail");

  arch::hcf();
}
