#include <std/memory.hpp>
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

extern "C" void kmain() {
  cxxabi::call_global_ctors();
  module::initialize_some(module::module_init_order::urgent);

  // arch::tss tss;

  arch::hcf();
}
