#include <arch/irq/irq_dispatcher.hpp>

extern uint64_t g_isrStubTable[arch::idt::ENTRY_COUNT];

namespace arch::irq {
std::array<irq_dispatcher::irq_handler_t, MAX_IRQS> irq_dispatcher::s_entries{};

void irq_dispatcher::install_handlers() {
  for (std::size_t i = 0; i < MAX_IRQS; i++) {
    irq_handler_t handler = s_entries[i];
    std::uint64_t isrStub = g_isrStubTable[i];

    idt::set_gate(static_cast<std::uint8_t>(i),
                  reinterpret_cast<idt::handler_t>(isrStub));
  }
}

void irq_dispatcher::set(std::uint8_t vec, irq_handler_t handler) {
  s_entries[vec] = handler;
}

void irq_dispatcher::dispatch(interrupt_frame *frame) {
  std::uint64_t num = frame->int_no;

  if (num >= MAX_IRQS)
    return;

  irq_handler_t handler = s_entries[num];
  if (handler)
    handler(frame);
}
} // namespace arch::irq
