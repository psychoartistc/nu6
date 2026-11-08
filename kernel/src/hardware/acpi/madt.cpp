#include <hardware/acpi/madt.hpp>
#include <hardware/controllers/ioapic.hpp>

namespace hardware::acpi {
bool madt_parser::parse(table_header *hdr) {
  madt_table *table = reinterpret_cast<madt_table *>(hdr);
  std::uintptr_t madtPtr = reinterpret_cast<std::uintptr_t>(table);

  std::uintptr_t end = madtPtr + table->header.length;
  std::uintptr_t current = reinterpret_cast<std::uintptr_t>(table->entries);

  while (current < end) {
    madt_entry_header *entry = reinterpret_cast<madt_entry_header *>(current);

    if (entry->type == IOAPIC_CODE)
      ctl::ioapic::all().emplace_back(*reinterpret_cast<madt_ioapic *>(entry));

    if (entry->length == 0)
      break;

    current += entry->length;
  }

  assert(ctl::ioapic::all().size() > 0 && "no ioapics found, fatal");
}

} // namespace hardware::acpi
