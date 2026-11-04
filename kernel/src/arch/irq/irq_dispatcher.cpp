#include <arch/irq/irq_dispatcher.hpp>
#include <arch/tables/ist.hpp>
#include <arch/irq/exceptions.hpp>

extern "C" {
extern uint64_t g_isrStubTable[arch::idt::ENTRY_COUNT];
}

namespace arch::irq {
std::array<irq_dispatcher::irq_handler_t, MAX_IRQS> irq_dispatcher::s_entries{};

static inline constexpr bool is_ist_irq(std::uint64_t num) {
  for (int i = 0; i < exceptions::IST_EXCEPTIONS_NUM; i++) {
    std::uint64_t istNum = exceptions::IST_EXCEPTIONS_INT_NO[i];
    if (istNum == num)
      return true;
  }

  return false;
}

void irq_dispatcher::install_handlers() {
  // this wraps once this reaches ist::STACK_COUNT - 1
  int currentIstIdx = 0;

  for (std::size_t i = 0; i < MAX_IRQS; i++) {
    irq_handler_t handler = s_entries[i];
    std::uint64_t isrStub = g_isrStubTable[i];

    if (is_ist_irq(i)) {
      idt::set_gate(static_cast<std::uint8_t>(i),
                    reinterpret_cast<idt::handler_t>(isrStub), 0x08,
                    currentIstIdx, 0x8E);

      currentIstIdx++;
      if (currentIstIdx == ist::STACK_COUNT)
        currentIstIdx = 0;
    } else
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

extern "C" {
void irq_dispatch(arch::interrupt_frame *frame) {
  arch::irq::irq_dispatcher::dispatch(frame);
}
}
