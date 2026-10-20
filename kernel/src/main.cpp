#include <stdnoreturn.h>

void kmain() {
  for (;;)
    asm volatile("hlt");
}
