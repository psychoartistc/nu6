#include <proc/timers/pit.hpp>
#include <std/stubs/uint128.hpp>
#include <arch/io/ports.hpp>
#include <arch/tsc.hpp>

namespace proc::timers {
void pit::disable() {
  arch::io::outb(0x43, 0x30);
  arch::io::outb(0x40, 0x01);
  arch::io::outb(0x40, 0x00);
}

std::uint64_t pit::tsc_herz() {
  std::uint64_t best_delta = ~0ULL;

  for (int i = 0; i < 5; i++) {

    arch::io::outb(0x61, (arch::io::inb(0x61) & ~0x02) | 0x01);

    arch::io::outb(0x43, 0xB0);
    arch::io::outb(0x42, CAL_COUNT & 0xFF);
    arch::io::outb(0x42, CAL_COUNT >> 8);

    std::uint64_t start = arch::tsc::read();
    while (!(arch::io::inb(0x61) & 0x20))
      ;
    std::uint64_t end = arch::tsc::read();

    if (end - start < best_delta)
      best_delta = end - start;
  }

  return static_cast<std::uint64_t>(
      (static_cast<std::uint128_t>(best_delta) * HERZ) / CAL_COUNT);
}
} // namespace proc::timers
