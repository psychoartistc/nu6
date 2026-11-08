#include <hardware/controllers/ioapic.hpp>

namespace hardware::ctl {
ioapic::ioapic(acpi::madt_ioapic entry) : m_madtEntry(entry) {}

std::vector<ioapic> &ioapic::all() { return s_ioapics; }

std::vector<ioapic> ioapic::s_ioapics{};
} // namespace hardware::ctl
