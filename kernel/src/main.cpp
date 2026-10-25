#include <std/memory.hpp>
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
#include <arch/tables/ist.hpp>
#include <mm/physical/freelist/freelist.hpp>

extern "C" void kmain() {
  cxxabi::call_global_ctors();
  module::initialize_some(module::module_init_order::urgent);

  mm::page_allocator::initialize();

  auto res = mm::page_allocator::allocate(1);
  if (!res)
    pr_error("err: %d", static_cast<int>(res.error()));
  else
    pr_info("0x%p", *res);

  arch::hcf();
}
