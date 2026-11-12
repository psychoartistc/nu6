#pragma once

#include <limine.h>
#include <arch/stack.hpp>
#include <cstdint>
#include <arch/tables/gdt.hpp>
#include <arch/tables/idt.hpp>
#include <arch/tables/ist.hpp>
#include <hardware/controllers/lapic.hpp>
#include <arch/tables/tss.hpp>

namespace arch::smp {
class core {
private:
  static inline constexpr std::uint64_t MSR_GS_BASE = 0xC0000101;

private:
  tss m_tss{};
  gdt m_gdt{};
  ist m_ist{};

  hardware::ctl::lapic m_lapic{};

  stack m_stack{0x0};

  std::uint64_t m_index{0};
  std::uint64_t m_lapicId{0};

  bool m_bBsp = false;

public:
  core(std::uint64_t idx, std::uint64_t lapic)
      : m_index(idx), m_lapicId(lapic) {}

  core() = default;

  std::uint64_t lapic_id() const { return m_lapicId; }
  std::uint64_t index() const { return m_index; }
  const stack &stack() const { return m_stack; }

  void set_current();

  // ran in core_manager
  void on_launch(bool bsp = false);

  // ran in on_launch after all the initialization
  void on_start();

  static core &current();
};

} // namespace arch::smp
