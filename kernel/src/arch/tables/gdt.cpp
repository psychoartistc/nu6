#include <arch/tables/gdt.hpp>

namespace arch {

void gdt::set_gate(std::size_t num, std::uint32_t base, std::uint32_t limit,
                   std::uint8_t access, std::uint8_t gran) {
  m_gdt[num].base_low = static_cast<std::uint16_t>(base & 0xFFFF);
  m_gdt[num].base_middle = static_cast<std::uint8_t>((base >> 16) & 0xFF);
  m_gdt[num].base_high = static_cast<std::uint8_t>((base >> 24) & 0xFF);

  m_gdt[num].limit_low = static_cast<std::uint16_t>(limit & 0xFFFF);

  m_gdt[num].granularity =
      static_cast<std::uint8_t>(((limit >> 16) & 0x0F) | (gran & 0xF0));

  m_gdt[num].access = access;
}

void gdt::set_tss_gate(std::size_t num, std::uint64_t base,
                       std::uint32_t limit) {
  set_gate(num, (std::uint32_t)base, limit, 0x89, 0x00);

  std::uint8_t *upper = (std::uint8_t *)&m_gdt[num + 1];
  upper[0] = (std::uint8_t)(base >> 32);
  upper[1] = (std::uint8_t)(base >> 40);
  upper[2] = (std::uint8_t)(base >> 48);
  upper[3] = (std::uint8_t)(base >> 56);
  upper[4] = 0;
  upper[5] = 0;
  upper[6] = 0;
  upper[7] = 0;
}

void gdt::initialize_and_load(std::uint64_t tss_base, std::uint32_t tss_limit) {
  m_gdtPtr.limit = sizeof(m_gdt) - 1;
  m_gdtPtr.base = reinterpret_cast<std::uint64_t>(m_gdt.data());

  set_gate(0, 0, 0, 0, 0);

  set_gate(1, 0, 0, 0x9A, 0x20);

  set_gate(2, 0, 0, 0x92, 0x00);

  set_tss_gate(TSS_INDEX, tss_base, tss_limit);

  set_gate(5, 0, 0, 0xF2, 0x00);
  set_gate(6, 0, 0, 0xFA, 0x20);

  asm volatile("lgdt %0\n\t"

               "mov $0x10, %%ax\n\t"
               "mov %%ax, %%ds\n\t"
               "mov %%ax, %%ss\n\t"
               "mov %%ax, %%es\n\t"
               "mov %%ax, %%fs\n\t"

               "pushq $0x08\n\t"
               "leaq 1f(%%rip), %%rax\n\t"
               "pushq %%rax\n\t"
               "lretq\n\t"
               "1:\n\t"
               :
               : "m"(m_gdtPtr)
               : "rax", "memory");

  m_logger.info("initialized");
}

void gdt::reload() { asm volatile("lgdt %0" : : "m"(m_gdtPtr) : "memory"); }

} // namespace arch
