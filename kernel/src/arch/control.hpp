#pragma once

namespace arch {
static inline void irq_disable() { asm volatile("cli"); }
static inline void irq_enable() { asm volatile("sti"); }
static inline void pause() { asm volatile("pause"); }
} // namespace arch
