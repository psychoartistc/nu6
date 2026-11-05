#include <std/memory.hpp>
#include <mm/kernel_mm.hpp>
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
#include <mm/virtual/slub_backend.hpp>
#include <abi/itanium.hpp>
#include <arch/tables/tss.hpp>
#include <arch/tables/idt.hpp>
#include <arch/tables/ist.hpp>
#include <std/dsa/rbtree.hpp>
#include <mm/virtual/page_table.hpp>
#include <mm/stl.hpp>
#include <utils/rng.hpp>
#include <mm/virtual/aslr.hpp>

extern "C" void kmain() {
  cxxabi::call_global_ctors();
  module::initialize_some(module::module_init_order::urgent);
  utils::seed_generator::initialize();
  mm::aslr_manager::initialize();

  bool r = mm::page_allocator::initialize();
  if (!r)
    utils::panic("failed to initialize page allocator");

  arch::smp::core_manager::launch_all();

  // removing these four lines would crash
  auto &pt = mm::get_kernel_pt();
  auto &tree = mm::get_kernel_vma_tree();
  (void)pt;
  (void)tree;

  utils::init_phase::checkpoint(utils::e_init_states::allocator_ready);

  arch::smp::core::current().on_launch(true);

  arch::hcf();
}
