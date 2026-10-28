#include <std/memory.hpp>
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

  mm::page_table pt;
  pt.clone_bootloader_pages();
  pt.switch_to();

  auto res = mm::page_allocator::allocate(1);
  if (!res)
    pr_error("err: %d", static_cast<int>(res.error()));
  else
    pr_info("0x%p", *res);

  mm::vma_tree<mm::pool_allocator<mm::vma>> tree{mm::KERNEL_VMA_START,
                                                 mm::KERNEL_VMA_END};

  auto vma = tree.allocate(4096);
  pr_info("allocated range 0x%p -> 0x%p", vma->address,
          vma->address + vma->count * mm::PAGE_SIZE);

  auto res3 = pt.map(*res, vma->address,
                     mm::protection::kernel | mm::protection::read |
                         mm::protection::write);

  pr_info("writing int %d to virtual address at 0x%p", 5, vma->address);
  int *a = reinterpret_cast<int *>(vma->address);
  *a = 5;
  pr_info("virtual address 0x%p has int value %d", vma->address, *a);

  arch::hcf();
}
