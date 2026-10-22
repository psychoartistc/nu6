
#include <stdbool.h>
#include <stdint.h>
#include <stdnoreturn.h>
#include <std/spinlock.hpp>

extern "C" void kmain() {

  for (;;)
    asm volatile("hlt");
}
