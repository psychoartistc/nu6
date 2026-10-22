#pragma once

namespace arch {
static inline constexpr void irq_disable() { asm volatile("cli"); }
static inline constexpr void irq_enable() { asm volatile("sti"); }
static inline constexpr void pause() { asm volatile("pause"); }
static inline constexpr void irq_wait() { asm volatile("hlt"); }
static inline constexpr void hcf() {
  for (;;)
    irq_wait();
}
} // namespace arch
