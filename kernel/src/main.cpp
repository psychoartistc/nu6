#include <std/memory.hpp>
#include <stdbool.h>
#include <stdint.h>
#include <stdnoreturn.h>
#include <std/spinlock.hpp>
#include <abi/itanium.hpp>

extern "C" void kmain() {
  cxxabi::call_global_ctors();

  char a[4];
  memset(a, 0, 4);

  for (;;)
    asm volatile("hlt");
}
