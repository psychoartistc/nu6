#include <hardware/controllers/ioapic.hpp>

namespace hardware::ctl {
ioapic::ioapic(acpi::madt_ioapic entry) : m_madtEntry(entry) {}
} // namespace hardware::ctl
