#include <arch/tables/tss.hpp>

namespace arch {

void tss::initialize() {
  m_tss = {};

  m_tss.io_map_base = sizeof(m_tss);

  m_logger.info("initialized");
}

void tss::set_ring0_stack(std::uint64_t stack_top) { m_tss.rsp[0] = stack_top; }

void tss::set_ist(std::uint8_t ist_index, std::uint64_t stack_top) {
  if (ist_index == 0 || ist_index > ist_count) {
    return;
  }

  m_tss.ist[ist_index - 1] = stack_top;
}

void tss::load(std::uint16_t selector) {
  asm volatile("ltr %0" : : "rm"(selector) : "memory");
  // m_logger.info("ltr succeeded");
}

} // namespace arch
