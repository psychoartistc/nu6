#include <arch/irq/exceptions.hpp>
#include <utility>
#include <utils/panic.hpp>
#include <arch/irq/irq_dispatcher.hpp>

namespace arch::irq {
void exceptions::install() {
  for (std::size_t i = 0; i < EXCEPTION_NUM; i++) {
    irq_dispatcher::set(
        static_cast<std::uint8_t>(i), [](interrupt_frame *frame) {
          std::uint64_t num = frame->int_no;
          utils::panic("cpu exception: %s", EXCEPTION_NAMES[num].data());
          std::unreachable();
        });
  }
}
} // namespace arch::irq
