#include <arch/irq/exceptions.hpp>
#include <arch/irq/irq_dispatcher.hpp>

namespace arch::irq {
void exceptions::install() {
  for (std::size_t i = 0; i < EXCEPTION_NUM; i++) {
    irq_dispatcher::set(static_cast<std::uint8_t>(i),
                        [](interrupt_frame *frame) {

                        });
  }
}
} // namespace arch::irq
