#pragma once

namespace arch {
static inline void irq_disable() { asm volatile("cli"); }
static inline void irq_enable() { asm volatile("sti"); }
static inline void pause() { asm volatile("pause"); }
static inline void irq_wait() { asm volatile("hlt"); }
static inline void hcf() {
  for (;;)
    irq_wait();
}
} // namespace arch
