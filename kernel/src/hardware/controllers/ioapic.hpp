#pragma once

#include <cstddef>
#include <cstdint>
#include <hardware/acpi/madt.hpp>

namespace hardware::ctl {
class ioapic {
private:
  acpi::madt_ioapic m_madtEntry{0x0};

public:
  // ioapic's are discovered through the madt table
  ioapic(acpi::madt_ioapic m);

  const acpi::madt_ioapic &madt_entry() const { return m_madtEntry; }
};
} // namespace hardware::ctl
