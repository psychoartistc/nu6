#pragma once

#include <std/vendor/veque.hpp>
#include <cstddef>
#include <cstdint>
#include <hardware/acpi/madt.hpp>

namespace hardware::ctl {
class ioapic {
private:
  static std::vector<ioapic> s_ioapics;

  acpi::madt_ioapic m_madtEntry{};

public:
  // ioapic's are discovered through the madt table
  ioapic(acpi::madt_ioapic m);

  const acpi::madt_ioapic &madt_entry() const { return m_madtEntry; }

  static std::vector<ioapic> &all();
};
} // namespace hardware::ctl
