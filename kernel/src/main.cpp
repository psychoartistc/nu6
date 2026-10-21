
#include <stdbool.h>
#include <stdint.h>
#include <stdnoreturn.h>

extern "C" void kmain() {

  for (;;)
    asm volatile("hlt");
}
