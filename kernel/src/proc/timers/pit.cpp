#include <proc/timers/pit.hpp>
#include <arch/io/ports.hpp>

namespace proc::timers {
void pit::disable() {
  arch::io::outb(0x43, 0x30);
  arch::io::outb(0x40, 0x01);
  arch::io::outb(0x40, 0x00);
}
} // namespace proc::timers
