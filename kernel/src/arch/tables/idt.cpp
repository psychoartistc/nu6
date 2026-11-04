#include <arch/tables/idt.hpp>

namespace arch {

bool idt::set_gate(std::uint8_t vector, handler_t handler,
                   std::uint16_t selector, std::uint8_t ist,
                   std::uint8_t type_attr) {
  if (handler == nullptr || ist > 7) {
    return false;
  }

  auto address = reinterpret_cast<std::uint64_t>(handler);

  auto &gate = s_idt[vector];

  gate.offset_low = static_cast<std::uint16_t>(address & 0xFFFF);

  gate.selector = selector;

  gate.ist = static_cast<std::uint8_t>(ist & 0x07);
  gate.type_attr = type_attr;

  gate.offset_middle = static_cast<std::uint16_t>((address >> 16) & 0xFFFF);

  gate.offset_high = static_cast<std::uint32_t>((address >> 32) & 0xFFFFFFFF);

  gate.reserved = 0;

  return true;
}

void idt::clear_gate(std::uint8_t vector) { s_idt[vector] = {}; }

void idt::initialize() {
  s_idt.fill({});

  s_idtPtr.limit = sizeof(s_idt) - 1;
  s_idtPtr.base = reinterpret_cast<std::uint64_t>(s_idt.data());

  // s_logger.info("initialized");
}

void idt::initialize_and_load() {
  initialize();
  reload();
}

void idt::reload() {
  asm volatile("lidt %0" : : "m"(s_idtPtr) : "memory");
  // s_logger.info("reloaded");
}

std::array<idt::entry, idt::ENTRY_COUNT> idt::s_idt{0};
idt::ptr idt::s_idtPtr{0x0};
logging::named_logger idt::s_logger{"idt"};

} // namespace arch
