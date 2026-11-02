#pragma once

#include <arch/tables/idt.hpp>
#include <functional>
#include <array>
#include <arch/irq/common.hpp>

namespace arch::irq {
class irq_dispatcher {
public:
  using irq_handler_t = std::function<void(interrupt_frame *)>;

  static std::array<irq_handler_t, MAX_IRQS> s_entries;

public:
  static void install_handlers();
  static void set(std::uint8_t vec, irq_handler_t handler);
  static void dispatch(interrupt_frame *frame);
};
} // namespace arch::irq

extern "C" {
void irq_dispatch(arch::interrupt_frame *frame);
}
